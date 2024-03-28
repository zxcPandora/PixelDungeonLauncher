#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardPaths>
#include <QMovie>
#include <QThread>
#include <QElapsedTimer>
#define	FILE_MAX_SIZE 1024

//启动器设置
bool shouldDownloadGame = false;
bool shouldDownloadProgram = false;
bool shouldAutoLaunchGame = true;
bool shouldAutoUpdateGame = true;
bool shouldAutoUpdateLauncher = true;

//下载相关
bool isDownloadingFile = false;
qint64 lastByteRead =0;
QMap<QString,QString> downloadFileList;
QElapsedTimer downloadTime;
QString speedText;
QString leftTimeText;

//Json相关
QString gameChangeLog;
QString gameDownloadLink;
QString programChangeLog;
QString programDownloadLink;

//其他
QString gameName;
QString LastestProgramVersion;
int LastestGameVersion;
int CurrentGameVersion;
QString testbat =QString()+
    "tasklist|find /i \"PixelDungeonLauncher.exe\">nul\n"+
    "if %errorlevel%==0 (\n"+
        "taskkill /F /T /IM \"PixelDungeonLauncher.exe\">nul\n"+
    ")\n"+
    "del /f /q .\\PixelDungeonLauncher.exe && move .\\Cache\\PixelDungeonLauncher.exe .\\ && start .\\PixelDungeonLauncher.exe\n";

//暂时无用
QString jarPath;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()& ~Qt::WindowMaximizeButtonHint);
    setFixedSize(this->width(), this->height());
    ui->btnTest->setVisible(false);
    ui->progressBar->setRange(0,100);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->actionAutoLaunchGame->setCheckable(true);
    ui->actionAutoUpdateGame->setCheckable(true);
    ui->actionAutoUpdateLauncher->setCheckable(true);
    this->setWindowTitle(tr("Launcher")+QCoreApplication::applicationVersion());

    connect(ui->btnLauncher,SIGNAL(clicked()),this,SLOT(btnLauncherGameClicked()));
    connect(ui->btnUpdateGame,SIGNAL(clicked()),this,SLOT(btnUpdateGameClicked()));
    connect(ui->btnTest,SIGNAL(clicked()),this,SLOT(btnTestClicked()));
    connect(ui->actionAutoLaunchGame,SIGNAL(triggered(bool)),this,SLOT(actionAutoLaunchGameTriggered(bool)));
    connect(ui->actionAutoUpdateGame,SIGNAL(triggered(bool)),this,SLOT(actionAutoUpdateGameTriggered(bool)));
    connect(ui->actionAutoUpdateLauncher,SIGNAL(triggered(bool)),this,SLOT(actionAutoUpdateLauncherTriggered(bool)));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(actionAboutTriggered()));
    connect(ui->actionUpdate,SIGNAL(triggered()),this,SLOT(actionUpdateTriggered()));

    LoadProgramSetting();
    CheckJava();
    ProcessGameJsonData();
    ProcessProgramJsonData();
    CheckGame();
    CheckProgram();

    if(shouldAutoLaunchGame && !shouldDownloadGame && !gameName.isEmpty()){
        QThread* thread = new QThread;
        MyThread* reallyThread = new MyThread;
        reallyThread->moveToThread(thread);
        PublicVariables::SetThreadCommand(QStringList()<<" java -jar "<<gameName<<"\r\n");
        thread->start();
        connect(this,&MainWindow::OnWindowLoadFinish,reallyThread,&MyThread::ThreadFunction);
    }

    connect(this,SIGNAL(OnWindowLoadFinish()),this,SLOT(OnWindowLoadFinished()));
    connect(this,SIGNAL(sigProgress(qint64, qint64, qreal)),this,SLOT(sltProgress(qint64, qint64, qreal)));
    connect(this,SIGNAL(sigDownloadFinished()),this,SLOT(sltDownloadFinished()));
    emit OnWindowLoadFinish();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::speed(double speed)
{
    QString strUnit;
    if (speed <= 0)
    {
        speed = 0;
        strUnit = "Bytes/S";
    }
    else if (speed < 1024)
    {
        strUnit = "Bytes/S";
    }
    else if (speed < 1024 * 1024)
    {
        speed /= 1024;
        strUnit = "KB/S";
    }
    else if (speed < 1024 * 1024 * 1024)
    {
        speed /= (1024 * 1024);
        strUnit = "MB/S";
    }
    else
    {
        speed /= (1024 * 1024 * 1024);
        strUnit = "GB/S";
    }

    QString strSpeed = QString::number(speed, 'f', 2);
    return QString("%1 %2").arg(strSpeed).arg(strUnit);
}

QString MainWindow::timeFormat(int seconds)
{
    QString strValue;
    QString strSpacing(" ");
    if (seconds <= 0)
    {
        strValue = QString("%1"+tr("seconds")).arg(0);
    }
    else if (seconds < 60)
    {
        strValue = QString("%1"+tr("seconds")).arg(seconds);
    }
    else if (seconds < 60 * 60)
    {
        int nMinute = seconds / 60;
        int nSecond = seconds - nMinute * 60;

        strValue = QString("%1"+tr("minutes")).arg(nMinute);

        if (nSecond > 0)
            strValue += strSpacing + QString("%1"+tr("seconds")).arg(nSecond);
    }
    else if (seconds < 60 * 60 * 24)
    {
        int nHour = seconds / (60 * 60);
        int nMinute = (seconds - nHour * 60 * 60) / 60;
        int nSecond = seconds - nHour * 60 * 60 - nMinute * 60;

        strValue = QString("%1"+tr("hours")).arg(nHour);

        if (nMinute > 0)
            strValue += strSpacing + QString("%1"+tr("minutes")).arg(nMinute);

        if (nSecond > 0)
            strValue += strSpacing + QString("%1"+tr("seconds")).arg(nSecond);
    }
    else
    {
        int nDay = seconds / (60 * 60 * 24);
        int nHour = (seconds - nDay * 60 * 60 * 24) / (60 * 60);
        int nMinute = (seconds - nDay * 60 * 60 * 24 - nHour * 60 * 60) / 60;
        int nSecond = seconds - nDay * 60 * 60 * 24 - nHour * 60 * 60 - nMinute * 60;

        strValue = QString("%1"+tr("days")).arg(nDay);

        if (nHour > 0)
            strValue += strSpacing + QString("%1h="+tr("hours")).arg(nHour);

        if (nMinute > 0)
            strValue += strSpacing + QString("%1"+tr("minutes")).arg(nMinute);

        if (nSecond > 0)
            strValue += strSpacing + QString("%1"+tr("seconds")).arg(nSecond);
    }

    return strValue;
}

void MainWindow::btnLauncherGameClicked()
{
    if(CheckGameRunningStatus())
        return;
}

void MainWindow::btnUpdateGameClicked()
{
    if(CurrentGameVersion >= LastestGameVersion){
        QMessageBox box;
        box.setWindowTitle(tr("TIP"));
        box.setText(tr("noUpdateText"));
        box.setIcon(QMessageBox::Information);
        box.setStandardButtons(QMessageBox::Yes);
        box.button(QMessageBox::Yes)->setText(tr("btnConfirm"));
        box.setDefaultButton(QMessageBox::Yes);
        box.exec();
        return;
    }

    if(CheckGameRunningStatus())
        return;

    ui->btnLauncher->setEnabled(false);
    ui->btnUpdateGame->setEnabled(false);
    QString gamePath = QCoreApplication::applicationDirPath();
    StartDownload(gameDownloadLink, gamePath,this);
}

void MainWindow::btnTestClicked()
{

}

void MainWindow::actionAutoLaunchGameTriggered(bool checked)
{
    SaveProgramSetting("AutoLaunchGame",checked);
}

void MainWindow::actionAutoUpdateGameTriggered(bool checked)
{
    SaveProgramSetting("AutoUpdateGame",checked);
}

void MainWindow::actionAutoUpdateLauncherTriggered(bool checked)
{
    SaveProgramSetting("AutoUpdateLauncher",checked);
}

void MainWindow::actionAboutTriggered()
{
    QMessageBox aboutBox;
    aboutBox.setText("<p align=\"center\">关于本软件的信息</p>");
    QString authorInfo = QString("<p>软件当前版本:"+QCoreApplication::applicationVersion()+"</p>")
                         + "<p align=\"center\"><b>作者信息</b></p>"
                         + "<p align=\"center\">zxcPandora</p>"
                         + "<p align=\"center\">Github: https://github.com/zxcPandora</p>"
                         + "<p align=\"center\">软件仓库: https://github.com/zxcPandora/PixelDungeonLauncher</p>";
    aboutBox.setInformativeText(authorInfo);
    aboutBox.setStandardButtons(QMessageBox::Yes);
    aboutBox.button(QMessageBox::Yes)->setText(tr("btnConfirm"));
    aboutBox.setDefaultButton(QMessageBox::Yes);
    aboutBox.exec();
}

void MainWindow::actionUpdateTriggered()
{
    if(HasNewProgramVersion()){
        QMessageBox box;
        box.setWindowTitle(tr("newProgramVersionTitle"));
        box.setText(tr("newProgramVersionText")+LastestProgramVersion);
        box.setInformativeText(programChangeLog);
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes|QMessageBox::Help|QMessageBox::No);
        box.button(QMessageBox::Yes)->setText(tr("btnWebSite"));
        box.button(QMessageBox::No)->setText(tr("btnCancel"));
        box.button(QMessageBox::Help)->setText(tr("btnDirectDownload"));
        box.setDefaultButton(QMessageBox::Yes);
        int selected = box.exec();
        switch(selected){
        case QMessageBox::Yes:
            QDesktopServices::openUrl(QUrl(QString("https://github.com/zxcPandora/PixelDungeonLauncher")));
            exit(0);
            break;
        case QMessageBox::Help:
            QString savePath = QCoreApplication::applicationDirPath()+"/Cache";
            if (!QFileInfo(savePath).isDir())
            {
                QDir dir;
                dir.mkpath(savePath);
            }
            ui->btnUpdateGame->setEnabled(false);
            StartDownload(programDownloadLink,savePath,this);
            break;
        }
    }
}

void MainWindow::sltProgress(qint64 bytesRead, qint64 totalBytes, qreal progress)
{
    ui->progressBar->setValue(progress*100);
    if(downloadTime.hasExpired(1000))
    {
        qint64 diff = bytesRead-lastByteRead;
        double dBytesSpeed = diff*1024 / 1000;
        double dSpeed = dBytesSpeed;
        qint64 leftBytes = (totalBytes - bytesRead);
        double dLeftTime = (leftBytes * 1.0) / dBytesSpeed;
        speedText = speed(dSpeed);
        leftTimeText = timeFormat(qCeil(dLeftTime));
        lastByteRead = bytesRead;
        downloadTime.restart();
    }


    QString title = QString::number(progress*100,'f',2)+"%  "
                    +QString::number(bytesRead/(1024.0*1024.0),'f',2)+"MB/"
                    +QString::number(totalBytes/(1024.0*1024.0),'f',2)+"MB "
                    +speedText
                    +tr("downloadLeftTime")+leftTimeText;
    ui->labelDownloadRate->setText(title);
    ui->labelDownloadStatus->setText(tr("Downloading")+PublicVariables::GetDownloadFileName());
}

void MainWindow::sltDownloadFinished()
{
    ui->labelDownloadStatus->setText(tr("DownloadFinished")+PublicVariables::GetDownloadFileName());
    ui->btnLauncher->setEnabled(true);
    ui->btnUpdateGame->setEnabled(true);
    isDownloadingFile = false;
    downloadFileList.remove(downloadFileList.firstKey());
    if(!downloadFileList.isEmpty()){
        StartDownload(downloadFileList.firstKey(),downloadFileList.first());
    }else if(HasNewProgramVersion()){
        QMessageBox box;
        box.setWindowTitle(tr("TIP"));
        box.setText(tr("programDownloadFinish"));
        box.setInformativeText(tr("programUpdateAndRestart"));
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes);
        box.button(QMessageBox::Yes)->setText(tr("btnConfirm"));
        box.setDefaultButton(QMessageBox::Yes);
        int result = box.exec();
        QCoreApplication::exit();
        QProcess::startDetached("LauncherUpdater.exe");
    }
}

void MainWindow::StartDownload(const QString &downloadUrl, const QString &savePath, QObject *parent)
{
    if(!downloadFileList.contains(downloadUrl))
        downloadFileList.insert(downloadUrl,savePath);

    if(isDownloadingFile)
        return;

    isDownloadingFile = true;
    DownloadFile* dT;
    dT = new DownloadFile(downloadUrl, savePath,parent);
    dT->startDownload();
    downloadTime.start();
}

void MainWindow::CreateProgramSetting()
{
    QString programPath = QCoreApplication::applicationDirPath();
    if(!QFile(programPath+"/LauncherSetting.ini").exists()){
        QSettings set(programPath+"/LauncherSetting.ini",QSettings::IniFormat);
        set.setValue("Version",QCoreApplication::applicationVersion());
        set.setValue("AutoLaunchGame",false);
        set.setValue("AutoUpdateGame",true);
        set.setValue("AutoUpdateLauncher",false);
        set.sync();
#ifndef Q_OS_WIN
        system("sync");
#endif
    }
    //bool result = QFile(programPath+"/LauncherSetting.ini").exists();
}

void MainWindow::SaveProgramSetting(QString key, QVariant value)
{
    QString programPath = QCoreApplication::applicationDirPath();
    if(!QFile(programPath+"/LauncherSetting.ini").exists()){
        CreateProgramSetting();
    }
    QSettings set(programPath+"/LauncherSetting.ini",QSettings::IniFormat);
    set.setValue(key,value);
    set.sync();
#ifndef Q_OS_WIN
    system("sync");
#endif
    LoadProgramSetting();
}

void MainWindow::LoadProgramSetting()
{
    QString programPath = QCoreApplication::applicationDirPath();
    if(!QFile(programPath+"/LauncherSetting.ini").exists()){
        CreateProgramSetting();
    }

    QSettings set(programPath+"/LauncherSetting.ini",QSettings::IniFormat);
    ui->actionAutoLaunchGame->setChecked(set.value("AutoLaunchGame",false).toBool());
    ui->actionAutoUpdateGame->setChecked(set.value("AutoUpdateGame",true).toBool());
    ui->actionAutoUpdateLauncher->setChecked(set.value("AutoUpdateLauncher",false).toBool());
    shouldAutoLaunchGame = set.value("AutoLaunchGame",false).toBool();
    shouldAutoUpdateGame = set.value("AutoUpdateGame",true).toBool();
    shouldAutoUpdateLauncher = set.value("AutoUpdateLauncher",false).toBool();
}

void MainWindow::OnWindowLoadFinished()
{
    if(shouldDownloadGame){
        if(CheckGameRunningStatus())
            return;
        ui->btnLauncher->setEnabled(false);
        ui->btnUpdateGame->setEnabled(false);
        QString gamePath = QCoreApplication::applicationDirPath();
        StartDownload(gameDownloadLink, gamePath,this);
    }
    if(shouldDownloadProgram){
        QString savePath = QCoreApplication::applicationDirPath()+"/Cache";
        if (!QFileInfo(savePath).isDir())
        {
            QDir dir;
            dir.mkpath(savePath);
        }
        ui->btnUpdateGame->setEnabled(false);
        StartDownload(programDownloadLink,savePath,this);
    }
    QThread* thread = new QThread;
    MyThread* reallyThread = new MyThread;
    reallyThread->moveToThread(thread);
    PublicVariables::SetThreadCommand(QStringList()<<" java -jar "<<gameName<<"\r\n");
    thread->start();;
    connect(ui->btnLauncher,&QPushButton::clicked,reallyThread,&MyThread::ThreadFunction);
    //RunClientCommand(QStringList()<<"/c"<<" java -jar "<<gamePath.append("/desktop-0.7.1.0.jar\r\n"));zz
}

void MainWindow::CheckJava()
{
    QString msg = RunClientCommand(QStringList()<<"java -version");

    if(!msg.contains("java version")){
        QMessageBox box;
        box.setWindowTitle(tr("checkJavaTitle"));
        box.setText(tr("checkJavaText"));
        box.setInformativeText(tr("checkJavaInformativeText"));
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        box.button(QMessageBox::Yes)->setText(tr("btnYes"));
        box.button(QMessageBox::No)->setText(tr("btnNo"));
        box.setDefaultButton(QMessageBox::Yes);
        int selected = box.exec();
        switch(selected){
        case QMessageBox::Yes:
            QDesktopServices::openUrl(QUrl(QString("https://www.oracle.com/java/technologies/downloads/#java8")));
            break;
        }
        exit(0);
    }
}

void MainWindow::CheckProgram()
{
    QString programPath = QCoreApplication::applicationDirPath();
    QDir dir(programPath);
    if(!dir.exists())
        return;

    bool hasNewVersion=false;
    if(shouldAutoUpdateLauncher){
        hasNewVersion = HasNewProgramVersion();
    }

    if(hasNewVersion){
        QMessageBox box;
        box.setWindowTitle(tr("newProgramVersionTitle"));
        box.setText(tr("newProgramVersionText")+LastestProgramVersion);
        box.setInformativeText(programChangeLog);
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes|QMessageBox::Help|QMessageBox::No);
        box.button(QMessageBox::Yes)->setText(tr("btnWebSite"));
        box.button(QMessageBox::No)->setText(tr("btnCancel"));
        box.button(QMessageBox::Help)->setText(tr("btnDirectDownload"));
        box.setDefaultButton(QMessageBox::Yes);
        int selected = box.exec();
        switch(selected){
        case QMessageBox::Yes:
            QDesktopServices::openUrl(QUrl(QString("https://github.com/zxcPandora/PixelDungeonLauncher")));
            exit(0);
            break;
        case QMessageBox::Help:
            shouldDownloadProgram = true;
            break;
        }
    }
}

void MainWindow::CheckGame()
{
    QString programPath = QCoreApplication::applicationDirPath();
    QDir dir(programPath);
    if(!dir.exists())
        return;

    if(CheckGameRunningStatus())
        return;

    QFileInfoList fileList = dir.entryInfoList(QStringList()<<"*.jar",QDir::Files|QDir::NoDotAndDotDot);
    bool gameExist=true;
    bool hasNewVersion=false;
    if(fileList.count()<=0){
        gameExist = false;
    }else if(shouldAutoUpdateGame){
        gameName = dir.absoluteFilePath(fileList[fileList.length()-1].fileName());
        GetGameVersion();
        hasNewVersion = CurrentGameVersion < LastestGameVersion;
    }

    if(!gameExist||hasNewVersion){
        QMessageBox box;
        box.setWindowTitle(gameExist?tr("newGameVersionTitle"):tr("checkGameTitle"));
        box.setText(gameExist?tr("newGameVersionText"):tr("checkGameText"));
        box.setInformativeText(gameChangeLog);
        box.setIcon(QMessageBox::Warning);
        box.setStandardButtons(QMessageBox::Yes|QMessageBox::Help|QMessageBox::No);
        box.button(QMessageBox::Yes)->setText(tr("btnWebSite"));
        box.button(QMessageBox::No)->setText(tr("btnCancel"));
        box.button(QMessageBox::Help)->setText(tr("btnDirectDownload"));
        box.setDefaultButton(QMessageBox::Yes);
        int selected = box.exec();
        switch(selected){
        case QMessageBox::Yes:
            QDesktopServices::openUrl(QUrl(QString("https://pd.qinyueqwq.top/?cat=8")));
            exit(0);
            break;
        case QMessageBox::Help:
            shouldDownloadGame = true;
            break;
        }
    }
}

bool MainWindow::CheckGameRunningStatus()
{
#ifdef Q_OS_WIN
    QString msg = RunClientCommand(QStringList()<<"wmic process where caption='javaw.exe' get processid,commandline");
#else
    QString msg = RunClientCommand(QStringList()<<"jps");
#endif

    if(!msg.contains(".jar")){
        return false;
    }

    QMessageBox box;
    box.setWindowTitle(tr("TIP"));
    box.setText(tr("gameRunning"));
    box.setInformativeText(tr("killGame"));
    box.setIcon(QMessageBox::Warning);
    box.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    box.button(QMessageBox::Yes)->setText(tr("btnYes"));
    box.button(QMessageBox::No)->setText(tr("btnNo"));
    box.setDefaultButton(QMessageBox::Yes);
    int selected = box.exec();
    switch(selected){
    case QMessageBox::No:
        return true;
    }
    box.close();

#ifdef Q_OS_WIN
    QStringList list = msg.split("\r\r\n");
#else
    QStringList list = msg.split("\r\n");
#endif
    for(int i =0;i<list.count();i++)
    {
        if(list[i].contains(".jar")){
#ifdef Q_OS_WIN
            RunClientCommand(QStringList()<<" taskkill /F /PID "<<list[i].mid(list[i].lastIndexOf('"')+1,list[i].length()-1).replace(" ",""));
#else
            RunClientCommand(QStringList()<<" kill -9 "<<list[i].split(" ")[0]);
#endif
        }
    }

    box.setWindowTitle(tr("TIP"));
    box.setText(tr("Finished"));
    box.setInformativeText(tr("alreadyKillGame"));
    box.setIcon(QMessageBox::Warning);
    box.setStandardButtons(QMessageBox::Yes);
    box.button(QMessageBox::Yes)->setText(tr("btnConfirm"));
    box.setDefaultButton(QMessageBox::Yes);
    box.exec();

    return false;
}

QString MainWindow::RunClientCommand(QStringList command)
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

void MainWindow::GetJarFilePath()
{
    if(CmdCheck())
        return;
    if(SystemEnvironmentCheck())
        return;
    if(RegeditCheck())
        return;
}

void MainWindow::GetGameVersion()
{
    QString gamePath = QCoreApplication::applicationDirPath();

    QZipReader zip(gameName);
    QTemporaryDir dir("Temp");
    bool ret =true;
    QVector<QZipReader::FileInfo> zipAllFiles = zip.fileInfoList();
    for (const QZipReader::FileInfo& zipFileInfo : zipAllFiles)
    {
        QString currDir2File = dir.path()+ "/" + zipFileInfo.filePath;//gamePath + "/" + zipFileInfo.filePath;
        if (zipFileInfo.isSymLink)
        {
            QString destination = QFile::decodeName(zip.fileData(zipFileInfo.filePath));
            if (destination.isEmpty())
            {
                ret = false;
                continue;
            }

            QFileInfo linkFi(currDir2File);
            if (!QFile::exists(linkFi.absolutePath()))
                QDir::root().mkpath(linkFi.absolutePath());
            if (!QFile::link(destination, currDir2File))
            {
                ret = false;
                continue;
            }
        }
        if (zipFileInfo.isDir)
        {
            continue;
            QDir(gamePath).mkpath(currDir2File);
        }
        if (zipFileInfo.isFile)
        {

            QByteArray dt = zipFileInfo.filePath.toUtf8();
            QString strtmp = QString::fromLocal8Bit(dt);


            if(!strtmp.contains("MANIFEST.MF"))
                continue;

            currDir2File.replace("/META-INF","");
            QFile currFile(currDir2File);
            if (!currFile.isOpen())
            {
                currFile.open(QIODevice::WriteOnly);
            }
            else {
                ret = false;
                continue;
            }

            qint64 size = zipFileInfo.size / 1024 / 1024;
            if (size > FILE_MAX_SIZE)
            {
                ret = false;
                continue;
            }
            QByteArray byteArr = zip.fileData(strtmp);
            currFile.write(byteArr);
            currFile.setPermissions(zipFileInfo.permissions);
            currFile.close();
        }
    }
    zip.close();
    qDebug()<<ret;
    QFile f(dir.path()+"\\MANIFEST.MF");
    f.open(QIODevice::ReadOnly);
    while(!f.atEnd()){
        QString text = f.readLine();
        if(text.contains("Implementation-Version"))
           CurrentGameVersion = QVariant(text.trimmed().replace("Implementation-Version: ","")).toInt();
    }
    f.close();
}

bool MainWindow::HasNewProgramVersion()
{
    if(!LastestProgramVersion.isEmpty()){
        QStringList version1List = QCoreApplication::applicationVersion().split(QLatin1Char('.'));
        QStringList version2List = LastestProgramVersion.split(QLatin1Char('.'));
        return VersionCompare(version1List,version2List);
    }
    return false;
}

bool MainWindow::VersionCompare(QStringList version1, QStringList version2)
{
    QVersionNumber v1;
    QVersionNumber v2;

    switch(version1.length()){
        case 1:
            v1 = QVersionNumber(version1[0].toInt());
            break;
        case 2:
            v1 = QVersionNumber(version1[0].toInt(),version1[1].toInt());
            break;
        case 3:
            v1 = QVersionNumber(version1[0].toInt(),version1[1].toInt(),version1[2].toInt());
            break;
    }
    switch(version1.length()){
        case 1:
            v2 = QVersionNumber(version2[0].toInt());
            break;
        case 2:
            v2 = QVersionNumber(version2[0].toInt(),version2[1].toInt());
            break;
        case 3:
            v2 = QVersionNumber(version2[0].toInt(),version2[1].toInt(),version2[2].toInt());
            break;
    }

    return v1.normalized() < v2.normalized();
}

bool MainWindow::CmdCheck()
{
    QString msg = RunClientCommand(QStringList()<<"java -verbose");
    if(msg.contains("loaded")){
        jarPath = msg.mid(msg.indexOf(":\\")-1,msg.indexOf("lib",msg.indexOf(":\\")-1)-msg.indexOf(":\\")).append("\\bin");
        return true;
    }
    return false;
}

bool MainWindow::SystemEnvironmentCheck()
{
    QStringList systemEnvironment = QProcess::systemEnvironment();
    foreach(QString path,systemEnvironment){
        if(path.contains("JAVA_HOME")){
            QString temp = QProcessEnvironment::systemEnvironment().value("JAVA_HOME").split(";")[0].append("\\bin");
            if(QFile(temp.append("\\java.exe")).exists()){
                jarPath = temp;
                return true;
            }
        }
    }

    QString systemPath = QProcessEnvironment::systemEnvironment().value("JAVA_HOME");
    QStringList list = systemPath.split(";");
    foreach (QString path, list) {
        if(path.contains("Java")){
            if(QFile(path.append("\\bin\\java.exe")).exists()){
                jarPath = path.append("\\bin");
                return true;
            }
        }
    }
    return false;
}

bool MainWindow::RegeditCheck()
{
    QSettings *set= new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit",QSettings::NativeFormat);
    QStringList list = set->childGroups();
    foreach (QString msg, list) {
        set = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\JavaSoft\\Java Development Kit\\"+msg,QSettings::NativeFormat);
        if(set->allKeys().contains("JavaHome")){
            jarPath = set->value("JavaHome").toString().append("\\bin");
            delete set;
            return true;
        }
    }
    delete set;
    return false;
}

void MainWindow::ProcessGameJsonData()
{
    QJsonDocument gameJson = PublicVariables::GetGameJson();
    if (!gameJson.isNull() && PublicVariables::GetGameJsonError().error == QJsonParseError::NoError){
        if(gameJson.isObject()){
            QJsonObject object = gameJson.object();
            // QStringList list = object.keys();
            // for(int i = 0; i < list.count(); i++)
            // {
            //     qDebug() << list.at(i);
            // }
            if(object.contains("DownloadLink3"))
            {
                gameDownloadLink = object["DownloadLink3"].toString();
            }
            if(object.contains("MLPDVersionCode"))
            {
                LastestGameVersion = object["MLPDVersionCode"].toString().toInt();
            }
            if(object.contains("changeLog"))
            {
                gameChangeLog = object["changeLog"].toString();
                ui->textEdit->setText(object["changeLog"].toString());
            }
        }
    }
}

void MainWindow::ProcessProgramJsonData()
{
    QJsonDocument programJson = PublicVariables::GetProgramJson();
    if (!programJson.isNull() && PublicVariables::GetProgramJsonError().error == QJsonParseError::NoError){
        QJsonDocument programJson = PublicVariables::GetProgramJson();
        QJsonArray object = programJson.array();
        QString tempVersion;

        QStringList version1List = QCoreApplication::applicationVersion().split(QLatin1Char('.'));
        QStringList version2List;
        QString tempDownloadLink;
        QString tempBody;
        for(int i=0;i<object.size();i++){
            QJsonObject tag = object.at(i).toObject();

            if(tag.contains("target_commitish") && !tag["target_commitish"].toString().contains("Magic-Ling"))
                continue;

            if(tag.contains("tag_name")
                    && (version2List = tag["tag_name"].toString().split(QLatin1Char('.'))).isEmpty()
                || !VersionCompare(tempVersion.split(QLatin1Char('.')),version2List)
                || !VersionCompare(version1List,version2List))
                continue;

            if(tag["assets"].toArray().isEmpty())
                continue;

            tempVersion = tag["tag_name"].toString();
            tempDownloadLink = tag["assets"].toArray()[1].toObject()["browser_download_url"].toString();
            tempBody = tag["body"].toString();
        }
        LastestProgramVersion = tempVersion;
        programDownloadLink = tempDownloadLink;
        programChangeLog = tempBody;
    }
}
