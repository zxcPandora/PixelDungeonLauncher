#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include "publicvariables.h"

class MyThread : public QObject
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = nullptr);
    void ThreadFunction();
signals:
};

#endif // MYTHREAD_H
