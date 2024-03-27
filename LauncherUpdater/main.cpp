#include <QCoreApplication>
#include <QLocale>
#include <QTranslator>
#include <QProcess>

QString RunClientCommand(QStringList command)
{
#ifdef Q_OS_WIN
    QString client = "cmd";
    command = QStringList()<<"/c"<<command;
#else
    QString client = "bash";
    command = QStringList()<<"-c"<<command;
#endif

    QString msg;
    QProcess progress;
    progress.setProcessChannelMode(QProcess::MergedChannels);
    progress.start(client, command);
    progress.waitForStarted();
    progress.waitForFinished();
    msg = QString::fromLocal8Bit(progress.readAllStandardOutput());
    progress.close();
    return msg;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "LauncherUpdater_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

#ifdef Q_OS_WIN
    QString msg = RunClientCommand(QStringList()<<"wmic process where caption='PixelDungeonLauncher.exe' get processid,commandline");
#else
    QString msg = RunClientCommand(QStringList()<<"idk the way in linux");
#endif
    if(msg.contains(".exe")){
#ifdef Q_OS_WIN
        QStringList list = msg.split("\r\r\n");
#else
        QStringList list = msg.split("\r\n");
#endif
        for(int i =0;i<list.count();i++)
        {
            if(list[i].contains(".exe")){
#ifdef Q_OS_WIN
                RunClientCommand(QStringList()<<" taskkill /F /PID "<<list[i].mid(list[i].lastIndexOf('"')+1,list[i].length()-1).replace(" ",""));
#else
                RunClientCommand(QStringList()<<" kill -9 "<<list[i].split(" ")[0]);
#endif
            }
        }
    }

#ifdef Q_OS_WIN
    msg = RunClientCommand(QStringList()<<"del /f /q .\\PixelDungeonLauncher.exe && move .\\Cache\\PixelDungeonLauncher.exe .\\ && start .\\PixelDungeonLauncher.exe\n");
#else
    msg = RunClientCommand(QStringList()<<"idk the way in linux");
#endif
    return 0;
    qDebug()<< msg;
    return a.exec();
}
