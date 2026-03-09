#include "crush_adapter.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

namespace AISDK {

CrushAdapter::CrushAdapter(QObject *parent)
    : CLIAdapter(parent)
    , m_process(new QProcess(this))
    , m_timeoutTimer(new QTimer(this))
{
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &CrushAdapter::onProcessReadyRead);
    connect(m_process, &QProcess::errorOccurred,
            this, &CrushAdapter::onProcessError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CrushAdapter::onProcessFinished);
    connect(m_process, &QProcess::stateChanged,
            this, [this](QProcess::ProcessState state) {
                emit processStateChanged(state);
            });

    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, &CrushAdapter::onTimeout);
}

CrushAdapter::~CrushAdapter()
{
    shutdown();
}

QString CrushAdapter::version() const
{
    if (!m_process || m_process->state() != QProcess::Running) {
        return QString();
    }

    // 通过 crush --version 获取版本
    QProcess versionProcess;
    versionProcess.start(m_config.executable, {QStringLiteral("--version")});
    if (versionProcess.waitForFinished(5000)) {
        return QString::fromUtf8(versionProcess.readAllStandardOutput()).trimmed();
    }
    return QString();
}

bool CrushAdapter::initialize(const AdapterConfig &config)
{
    m_config = config;

    // 检查可执行文件是否存在
    QFileInfo exeInfo(m_config.executable);
    if (!exeInfo.exists() && !m_config.executable.startsWith(QStringLiteral("crush"))) {
        qWarning() << "[CrushAdapter] Executable not found:" << m_config.executable;
        // 尝试在 PATH 中查找
        m_config.executable = QStringLiteral("crush");
    }

    qDebug() << "[CrushAdapter] Initializing with executable:" << m_config.executable;
    startProcess();
    return true;
}

bool CrushAdapter::isReady() const
{
    return m_ready && m_process && m_process->state() == QProcess::Running;
}

void CrushAdapter::shutdown()
{
    qDebug() << "[CrushAdapter] Shutting down";
    stopProcess();
    m_ready = false;
    emit readyChanged(false);
}

void CrushAdapter::chat(const QString &message,
                         const QJsonArray &history,
                         const QJsonObject &options)
{
    if (!isReady()) {
        setError(tr("Crush adapter is not ready"));
        return;
    }

    qDebug() << "[CrushAdapter] Sending chat message:" << message.left(100);

    m_timeoutTimer->start(m_config.timeoutMs);

    // 构建请求
    QJsonObject request;
    request[QStringLiteral("type")] = QStringLiteral("chat");
    request[QStringLiteral("content")] = message;

    if (!history.isEmpty()) {
        request[QStringLiteral("history")] = history;
    }

    if (!m_config.model.isEmpty()) {
        request[QStringLiteral("model")] = m_config.model;
    }

    // 合并选项
    for (auto it = options.begin(); it != options.end(); ++it) {
        request[it.key()] = it.value();
    }

    if (!m_systemPrompt.isEmpty()) {
        request[QStringLiteral("system")] = m_systemPrompt;
    }

    writeToProcess(request);
}

void CrushAdapter::cancel()
{
    qDebug() << "[CrushAdapter] Cancelling request";

    m_timeoutTimer->stop();

    if (m_process && m_process->state() == QProcess::Running) {
        // 发送取消请求
        QJsonObject cancelReq;
        cancelReq[QStringLiteral("type")] = QStringLiteral("cancel");
        writeToProcess(cancelReq);
    }
}

bool CrushAdapter::loadMCPConfig(const QString &configPath)
{
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[CrushAdapter] Failed to open MCP config:" << configPath;
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (doc.isNull()) {
        qWarning() << "[CrushAdapter] Failed to parse MCP config:" << error.errorString();
        return false;
    }

    m_mcpConfig = doc.object();
    qDebug() << "[CrushAdapter] Loaded MCP config from:" << configPath;

    // 如果进程已运行，发送配置更新
    if (isReady()) {
        QJsonObject request;
        request[QStringLiteral("type")] = QStringLiteral("config");
        request[QStringLiteral("mcp")] = m_mcpConfig;
        writeToProcess(request);
    }

    return true;
}

void CrushAdapter::setSystemPrompt(const QString &prompt)
{
    m_systemPrompt = prompt;
}

QStringList CrushAdapter::availableModels() const
{
    // Crush 支持的模型列表
    // 实际应该从 crush --list-models 获取
    return {
        // Ollama 模型
        QStringLiteral("ollama:qwen3.5:27b"),
        QStringLiteral("ollama:llama3.2"),
        QStringLiteral("ollama:codellama"),
        QStringLiteral("ollama:mistral"),
        // OpenAI 模型
        QStringLiteral("openai:gpt-4"),
        QStringLiteral("openai:gpt-4o"),
        QStringLiteral("openai:gpt-3.5-turbo"),
        // Anthropic 模型
        QStringLiteral("anthropic:claude-3-opus"),
        QStringLiteral("anthropic:claude-3-sonnet"),
    };
}

void CrushAdapter::startProcess()
{
    if (m_process->state() != QProcess::NotRunning) {
        qDebug() << "[CrushAdapter] Process already running";
        return;
    }

    QStringList args;
    args << QStringLiteral("--json-mode");  // JSON 模式，便于解析

    if (!m_config.model.isEmpty()) {
        args << QStringLiteral("--model") << m_config.model;
    }

    if (!m_config.provider.isEmpty()) {
        args << QStringLiteral("--provider") << m_config.provider;
    }

    args << m_config.extraArgs;

    qDebug() << "[CrushAdapter] Starting process:" << m_config.executable << args;

    m_process->start(m_config.executable, args);

    if (m_process->waitForStarted(5000)) {
        qDebug() << "[CrushAdapter] Process started successfully";
        m_ready = true;
        emit readyChanged(true);
    } else {
        qWarning() << "[CrushAdapter] Failed to start process:"
                   << m_process->errorString();
        setError(tr("Failed to start Crush: %1").arg(m_process->errorString()));
    }
}

void CrushAdapter::stopProcess()
{
    m_timeoutTimer->stop();

    if (m_process && m_process->state() != QProcess::NotRunning) {
        qDebug() << "[CrushAdapter] Stopping process";

        // 尝试优雅退出
        QJsonObject exitReq;
        exitReq[QStringLiteral("type")] = QStringLiteral("exit");
        writeToProcess(exitReq);

        if (!m_process->waitForFinished(3000)) {
            qDebug() << "[CrushAdapter] Force killing process";
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }

    m_ready = false;
}

void CrushAdapter::writeToProcess(const QJsonObject &message)
{
    if (!m_process || m_process->state() != QProcess::Running) {
        qWarning() << "[CrushAdapter] Cannot write - process not running";
        return;
    }

    QByteArray data = QJsonDocument(message).toJson(QJsonDocument::Compact) + "\n";
    m_process->write(data);
    // QProcess 会自动缓冲写入，无需手动 flush
}

void CrushAdapter::onProcessReadyRead()
{
    QByteArray data = m_process->readAllStandardOutput();
    m_buffer += QString::fromUtf8(data);

    // 按行处理
    int newlinePos;
    while ((newlinePos = m_buffer.indexOf(QLatin1Char('\n'))) != -1) {
        QString line = m_buffer.left(newlinePos);
        m_buffer.remove(0, newlinePos + 1);

        if (line.endsWith(QLatin1Char('\r'))) {
            line.chop(1);
        }

        if (!line.trimmed().isEmpty()) {
            processLine(line);
        }
    }
}

void CrushAdapter::processLine(const QString &line)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8(), &error);

    if (doc.isNull()) {
        // 可能是非 JSON 输出（如进度信息）
        qDebug() << "[CrushAdapter] Non-JSON line:" << line;
        return;
    }

    handleResponse(doc.object());
}

void CrushAdapter::handleResponse(const QJsonObject &response)
{
    QString type = response[QStringLiteral("type")].toString();

    if (type == QStringLiteral("ready")) {
        qDebug() << "[CrushAdapter] Received ready signal";
        m_ready = true;
        emit readyChanged(true);
    }
    else if (type == QStringLiteral("content") || type == QStringLiteral("text")) {
        // 文本内容
        QString text = response[QStringLiteral("content")].toString();
        if (text.isEmpty()) {
            text = response[QStringLiteral("text")].toString();
        }

        MessageChunk chunk;
        chunk.type = MessageChunk::Text;
        chunk.content = text;
        emit chunkReceived(chunk);
        emit partialTextReceived(text);
    }
    else if (type == QStringLiteral("thinking")) {
        // 思考内容
        QString thinking = response[QStringLiteral("content")].toString();
        MessageChunk chunk;
        chunk.type = MessageChunk::Thinking;
        chunk.content = thinking;
        emit chunkReceived(chunk);
        emit thinkingReceived(thinking);
    }
    else if (type == QStringLiteral("tool_call")) {
        // 工具调用
        QString id = response[QStringLiteral("id")].toString();
        QString name = response[QStringLiteral("name")].toString();
        QJsonObject input = response[QStringLiteral("input")].toObject();

        MessageChunk chunk;
        chunk.type = MessageChunk::ToolCall;
        chunk.id = id;
        chunk.name = name;
        chunk.input = input;
        emit chunkReceived(chunk);
        emit toolCallReceived(id, name, input);
    }
    else if (type == QStringLiteral("tool_result")) {
        // 工具结果
        QString id = response[QStringLiteral("id")].toString();
        QString result = response[QStringLiteral("result")].toString();
        bool isError = response[QStringLiteral("error")].toBool();

        MessageChunk chunk;
        chunk.type = MessageChunk::ToolResult;
        chunk.id = id;
        chunk.content = result;
        chunk.isError = isError;
        emit chunkReceived(chunk);
        emit toolResultReceived(id, result, isError);
    }
    else if (type == QStringLiteral("done") || type == QStringLiteral("complete")) {
        // 完成
        m_timeoutTimer->stop();

        QString fullText = response[QStringLiteral("full_content")].toString();

        MessageChunk chunk;
        chunk.type = MessageChunk::Done;
        emit chunkReceived(chunk);

        if (!fullText.isEmpty()) {
            emit fullResponseReceived(fullText);
        }
    }
    else if (type == QStringLiteral("error")) {
        QString errorMsg = response[QStringLiteral("message")].toString();
        if (errorMsg.isEmpty()) {
            errorMsg = response[QStringLiteral("error")].toString();
        }
        setError(errorMsg);
    }
    else {
        qDebug() << "[CrushAdapter] Unknown response type:" << type
                 << "data:" << QJsonDocument(response).toJson(QJsonDocument::Compact);
    }
}

void CrushAdapter::onProcessError(QProcess::ProcessError error)
{
    QString errorStr;
    switch (error) {
    case QProcess::FailedToStart:
        errorStr = tr("Failed to start Crush process");
        break;
    case QProcess::Crashed:
        errorStr = tr("Crush process crashed");
        break;
    case QProcess::Timedout:
        errorStr = tr("Crush process timed out");
        break;
    case QProcess::WriteError:
        errorStr = tr("Failed to write to Crush process");
        break;
    case QProcess::ReadError:
        errorStr = tr("Failed to read from Crush process");
        break;
    default:
        errorStr = tr("Unknown process error");
    }

    qWarning() << "[CrushAdapter] Process error:" << errorStr;
    setError(errorStr);
}

void CrushAdapter::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "[CrushAdapter] Process finished with exit code:" << exitCode
             << "status:" << exitStatus;

    m_ready = false;
    emit readyChanged(false);

    if (exitStatus == QProcess::CrashExit) {
        setError(tr("Crush process crashed with exit code %1").arg(exitCode));
    }
}

void CrushAdapter::onTimeout()
{
    qWarning() << "[CrushAdapter] Request timed out";
    setError(tr("Request timed out"));
    cancel();
}

void CrushAdapter::setError(const QString &error)
{
    qWarning() << "[CrushAdapter] Error:" << error;

    MessageChunk chunk;
    chunk.type = MessageChunk::Error;
    chunk.content = error;
    chunk.isError = true;
    emit chunkReceived(chunk);
    emit errorOccurred(error);
}

} // namespace AISDK
