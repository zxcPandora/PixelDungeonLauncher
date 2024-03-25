#ifndef DOWNLOADFILE_H
#define DOWNLOADFILE_H

#include <memory>
#include <QObject>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QPointer>
#include <QApplication>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include "mainwindow.h"
#include "publicvariables.h"

class MainWindow;
class DownloadFile : public QObject
{
    Q_OBJECT
public:
    // 参数1=下载链接 参数2=保存的路径 参数3=父窗口地址
    explicit DownloadFile(const QString& downloadUrl, const QString& savePath, QObject* parent = nullptr);

    ~DownloadFile();

    bool startDownload();  // 开始下载文件
    void cancelDownload(); // 取消下载文件
Q_SIGNALS:
    // 下载进度信号
    void sigProgress(qint64 bytesRead, qint64 totalBytes, qreal progress);
    // 下载完成信号
    void sigDownloadFinished();

private Q_SLOTS:
    // QNetworkReply::finished对应的槽函数
    void Finished();
    // QIODevice::readyRead对应的槽函数
    void ReadyRead();
    // QNetworkReply::downloadProgress对应的槽函数
    void networkReplyProgress(qint64 bytesRead, qint64 totalBytes);

private:
    void startRequest(const QUrl& requestedUrl);
    std::unique_ptr<QFile> openFileForWrite(const QString& fileName);

private:
    // 保存构造时传入的下载url
    QString m_downloadUrl;
    // 保存构造时传入的保存路径
    QString m_savePath;
    // 默认文件名称
    const QString defaultFileName = "pd.jar";
    QUrl url;
    QNetworkAccessManager qnam;
    QPointer<QNetworkReply> reply;
    std::unique_ptr<QFile> m_fileptr;
    bool RequestAborted;

    MainWindow *m_mainwindow;

};

#endif // DOWNLOADFILE_H
