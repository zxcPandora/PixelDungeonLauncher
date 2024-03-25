#include "publicvariables.h"

QJsonDocument PublicVariables::gameJson;
QJsonParseError PublicVariables::gameJsonError;
QJsonDocument PublicVariables::programJson;
QJsonParseError PublicVariables::programJsonError;
QString PublicVariables::downloadFileName;
QStringList PublicVariables::threadCommand;

PublicVariables::PublicVariables() {}

void PublicVariables::SetGameJson(QJsonDocument jsonDocument)
{
    PublicVariables::gameJson = jsonDocument;
}

void PublicVariables::SetGameJsonError(QJsonParseError jsonParseError)
{
    PublicVariables::gameJsonError = jsonParseError;
}

void PublicVariables::SetProgramJson(QJsonDocument jsonDocument)
{
    PublicVariables::programJson = jsonDocument;
}

void PublicVariables::SetProgramJsonError(QJsonParseError jsonParseError)
{
    PublicVariables::programJsonError = jsonParseError;
}

void PublicVariables::SetDownloadFileName(QString downloadFileName)
{
    PublicVariables::downloadFileName = downloadFileName;
}

void PublicVariables::SetThreadCommand(QStringList threadCommand)
{
    PublicVariables::threadCommand = threadCommand;
}

QJsonDocument PublicVariables::GetGameJson()
{
    return PublicVariables::gameJson;
}

QJsonParseError PublicVariables::GetGameJsonError()
{
    return PublicVariables::gameJsonError;
}

QJsonDocument PublicVariables::GetProgramJson()
{
    return PublicVariables::programJson;
}

QJsonParseError PublicVariables::GetProgramJsonError()
{
    return PublicVariables::programJsonError;
}

QString PublicVariables::GetDownloadFileName()
{
    return PublicVariables::downloadFileName;
}

QStringList PublicVariables::GetThreadCommand()
{
    return PublicVariables::threadCommand;
}
