#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "publicvariables.h"
#include "mythread.h"
#include "downloadfile.h"
#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QtGlobal>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QSettings>
#include <QProcessEnvironment>
#include <private/qzipreader_p.h>
#include <QTemporaryDir>
#include <QMap>
#include <QVersionNumber>
#include <QMutex>
#include <QCommandLineOption>
#include <QCommandLineParser>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString speed(double speed);
    QString timeFormat(int seconds);

signals:
    void OnWindowLoadFinish();

    void sigProgress(qint64 bytesRead, qint64 totalBytes, qreal progress);  // 下载进度信号
    void sigDownloadFinished();  // 下载完成信号

    private slots:
        void btnLauncherGameClicked();
        void btnUpdateGameClicked();
        void btnTestClicked();

        void actionAutoLaunchGameTriggered(bool checked);
        void actionAutoUpdateGameTriggered(bool checked);
        void actionAutoUpdateLauncherTriggered(bool checked);
        void actionAboutTriggered();

        void OnWindowLoadFinished();

        void sltProgress(qint64 bytesRead, qint64 totalBytes, qreal progress);  // 下载进度信号
        void sltDownloadFinished();  // 下载完成信号z


private:
    Ui::MainWindow *ui;
    void StartDownload(const QString& downloadUrl, const QString& savePath, QObject* parent = nullptr);

    void CreateProgramSetting();
    void SaveProgramSetting(QString key,QVariant value);
    void LoadProgramSetting();

    void CheckJava();
    void CheckProgram();
    void CheckGame();
    bool CheckGameRunningStatus();

    QString RunClientCommand(QStringList command);

    void GetJarFilePath();//暂时无用
    void GetGameVersion();
    bool HasNewProgramVersion();

    bool CmdCheck();
    bool SystemEnvironmentCheck();
    bool RegeditCheck();

    void ProcessGameJsonData();
    void ProcessProgramJsonData();
};
#endif // MAINWINDOW_H
