#include "downloadfile.h"

DownloadFile::DownloadFile(const QString& downloadUrl, const QString& savePath, QObject* parent)
    : QObject{parent}
{
    m_downloadUrl = downloadUrl;
    m_savePath    = savePath;
    m_mainwindow = (MainWindow*)parent;
}

DownloadFile::~DownloadFile()
{

}
bool DownloadFile::startDownload()
{
    const QUrl newUrl = QUrl::fromUserInput(m_downloadUrl);

    if (!newUrl.isValid()) {
        qDebug() << QString("Download failed: %1.").arg(reply->errorString());
        return false;
    }
    QString svrloadFile = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    QString fileName = newUrl.fileName();
    //判断文件名称是否正确
    if (fileName.isEmpty())
    {
        fileName = defaultFileName;
    }
    PublicVariables::SetDownloadFileName(fileName);
    //判断传有路径，没有则默认路径
    if (m_savePath.isEmpty())
    {
        m_savePath = svrloadFile;
    }
    //判断路径是否存在，不存在创建
    if (!QFileInfo(m_savePath).isDir())
    {
        QDir dir;
        dir.mkpath(m_savePath);
    }
    fileName.prepend(m_savePath + '/');
    if (QFile::exists(fileName))//判断是否存在此文件
    {
        QFile::remove(fileName);
    }
    m_fileptr = openFileForWrite(fileName);
    if (!m_fileptr)
        return false;

    startRequest(newUrl);

    return true;
}
void DownloadFile::cancelDownload()
{
    RequestAborted = true;
    reply->abort();
}
void DownloadFile::Finished()
{
    QFileInfo fi;
    if (m_fileptr) {
        fi.setFile(m_fileptr->fileName());
        m_fileptr->close();
        m_fileptr.reset();
    }

    if (RequestAborted) {
        return;
    }

    if (reply->error()) {
        QFile::remove(fi.absoluteFilePath());
        qDebug() << QString("Download failed: %1.").arg(reply->errorString());
        return;
    }

    const QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (!redirectionTarget.isNull()) {
        const QUrl redirectedUrl = url.resolved(redirectionTarget.toUrl());
        m_fileptr = openFileForWrite(fi.absoluteFilePath());
        if (!m_fileptr)
        {
            return;
        }
        startRequest(redirectedUrl);
        return;
    }

    Q_EMIT sigDownloadFinished();

    Q_EMIT m_mainwindow->sigDownloadFinished();
}

void DownloadFile::ReadyRead()
{
    if (m_fileptr)
        m_fileptr->write(reply->readAll());
}
void DownloadFile::networkReplyProgress(qint64 bytesRead, qint64 totalBytes)
{
    qreal progress = qreal(bytesRead) / qreal(totalBytes);
    Q_EMIT sigProgress(bytesRead, totalBytes, progress);
    //qDebug() << QString::number(progress * 100, 'f', 2) << "%    "
    //         << bytesRead / (1024 * 1024) << "MB" << "/" << totalBytes / (1024 * 1024) << "MB";
    emit m_mainwindow->sigProgress(bytesRead, totalBytes, progress);
}
void DownloadFile::startRequest(const QUrl& requestedUrl)
{
    try {
        url = requestedUrl;
        RequestAborted = false;
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setProtocol(QSsl::AnyProtocol);
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        QNetworkRequest request = QNetworkRequest(url);
        request.setSslConfiguration(config);
        reply = qnam.get(request);

        connect(reply, &QNetworkReply::finished, this, &DownloadFile::Finished);
        connect(reply, &QIODevice::readyRead, this, &DownloadFile::ReadyRead);
        connect(reply, &QNetworkReply::downloadProgress, this, &DownloadFile::networkReplyProgress);
        //qDebug() << QString(tr("Downloading %1...").arg(url.toString()));
    }
    catch (...)
    {
        return ;
    }

}
std::unique_ptr<QFile> DownloadFile::openFileForWrite(const QString& fileName)
{
    std::unique_ptr<QFile> file(new QFile(fileName));
    if (!file->open(QIODevice::WriteOnly))
    {
        qDebug() << QString("Unable to save the file %1: %2.")
                        .arg(QDir::toNativeSeparators(fileName), file->errorString());
        return nullptr;
    }
    return file;
}
