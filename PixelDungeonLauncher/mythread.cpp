#include "mythread.h"
#include <QCoreApplication>
#include <QProcess>

MyThread::MyThread(QObject *parent)
    : QObject{parent}
{}

void MyThread::ThreadFunction()
{
#ifdef Q_OS_WIN
    QString client = "cmd";
    QStringList command = QStringList()<<"/c"<<PublicVariables::GetThreadCommand();
#else
    QString client = "bash";
    QStringList command = QStringList()<<"-c"<<PublicVariables::GetThreadCommand();
#endif

    qDebug()<<"command:"<<command;
    QString msg;
    QProcess progress(this);
    progress.setProcessChannelMode(QProcess::MergedChannels);
    progress.start(client, command);
    progress.waitForStarted();
    progress.waitForFinished();
    msg = QString::fromLocal8Bit(progress.readAllStandardOutput());
    progress.close();
}
