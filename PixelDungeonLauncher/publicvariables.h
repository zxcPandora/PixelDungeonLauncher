#ifndef PUBLICVARIABLES_H
#define PUBLICVARIABLES_H

#include <QJsonDocument>
#include <QJsonParseError>
#include <QMutex>
#include <loading.h>



class PublicVariables
{
public:
    PublicVariables();
    static void SetGameJson(QJsonDocument jsonDocument);
    static void SetGameJsonError(QJsonParseError jsonParseError);
    static void SetProgramJson(QJsonDocument jsonDocument);
    static void SetProgramJsonError(QJsonParseError jsonParseError);
    static void SetDownloadFileName(QString downloadFileName);
    static void SetThreadCommand(QStringList threadCommand);

    static QJsonDocument GetGameJson();
    static QJsonParseError GetGameJsonError();
    static QJsonDocument GetProgramJson();
    static QJsonParseError GetProgramJsonError();
    static QString GetDownloadFileName();
    static QStringList GetThreadCommand();

private:
    static QJsonDocument gameJson;
    static QJsonParseError gameJsonError;
    static QJsonDocument programJson;
    static QJsonParseError programJsonError;
    static QString downloadFileName;
    static QStringList threadCommand;
};

#endif // PUBLICVARIABLES_H
