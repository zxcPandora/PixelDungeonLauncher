#include "mainwindow.h"
#include "loading.h"
#include "publicvariables.h"

#define LAUNCHER_VERSION "1.0.0"
#define LAUNCHER_NAME "PixelDungeonLauncher"

#ifdef Q_OS_WIN32   //for win
#include <windows.h>
bool checkOnly()
{
    //  创建互斥量
    HANDLE m_hMutex  =  CreateMutex(NULL, FALSE,  (LPCWSTR)LAUNCHER_NAME );
    //  检查错误代码
    if  (GetLastError()  ==  ERROR_ALREADY_EXISTS)  {
        //  如果已有互斥量存在则释放句柄并复位互斥量
        CloseHandle(m_hMutex);
        m_hMutex  =  NULL;
        //  程序退出
        return  false;
    }
    else
        return true;
}
#endif

#ifdef Q_OS_LINUX   //for linux
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
bool checkOnly()
{
    const char filename[]  = "/tmp/lockfile";
    int fd = open (filename, O_WRONLY | O_CREAT , 0644);
    int flock = lockf(fd, F_TLOCK, 0 );
    if (fd == -1) {
        perror("open lockfile/n");
        return false;
    }
    //给文件加锁
    if (flock == -1) {
        perror("lock file error/n");
        return false;
    }
    //程序退出后，文件自动解锁
    return true;
}
#endif

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setApplicationVersion(LAUNCHER_VERSION);
    QCoreApplication::setApplicationName(LAUNCHER_NAME);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "PixelDungeonLauncher_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    if(!checkOnly()){
        QMessageBox::warning(nullptr, QObject::tr("ERROR"), QObject::tr("SingleProgramCheck"));
        return 0;
    }

    Loading loading;
    loading.setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    loading.setWindowModality(Qt::ApplicationModal);
    loading.show();


    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl("https://rust.coldmint.top/ftp/ling/json/GameUpdate.json")));
    QByteArray responseData;
    QEventLoop eventLoop;
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    if(reply->error() != QNetworkReply::NoError)
    {
        QMessageBox box;
        box.setWindowTitle(QObject::tr("Error"));
        box.setText(QObject::tr("NetError"));
        box.setInformativeText(QVariant(reply->error()).toString());
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes);
        box.setDefaultButton(QMessageBox::Yes);
        box.button(QMessageBox::Yes)->setText(QObject::tr("btnConfirm"));
        box.exec();
        return 0;
    }
    responseData = reply->readAll();
    QJsonParseError error;
    PublicVariables::SetGameJson(QJsonDocument::fromJson(responseData,&error));
    PublicVariables::SetGameJsonError(error);

    // QEventLoop eventloop2;
    // QTimer::singleShot(1000, &eventloop2, SLOT(quit()));
    // eventloop2.exec();
    loading.close();



    MainWindow w;
    w.show();
    return a.exec();
}
