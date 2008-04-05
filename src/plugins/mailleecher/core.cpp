#include "core.h"
#include "workingthread.h"

Q_GLOBAL_STATIC (Core, CoreInstance);

Core* Core::Instance ()
{
    return CoreInstance ();
}

Core::Core (QObject *parent)
: QObject (parent)
{
    Thread_ = 0;
}

void Core::StartDownload (const QString& addr, int port, const QString& login, const QString& password, const QString& where)
{
    Thread_ = new WorkingThread (this);
    Thread_->SetHost (addr, port);
    Thread_->SetAuth (login, password);
    Thread_->SetDest (where);
    Thread_->start ();
    connect (Thread_, SIGNAL (error (const QString&)), this, SIGNAL (error (const QString&)));
    connect (Thread_, SIGNAL (log (const QString&)), this, SIGNAL (log (const QString&)));
    connect (Thread_, SIGNAL (finished (bool)), this, SLOT (handleFinish (bool)));
    connect (Thread_, SIGNAL (mailProgress (int)), this, SIGNAL (mailProgress (int)));
    connect (Thread_, SIGNAL (dataProgress (int)), this, SIGNAL (dataProgress (int)));
    connect (Thread_, SIGNAL (totalMail (int)), this, SIGNAL (totalMail (int)));
    connect (Thread_, SIGNAL (totalData (int)), this, SIGNAL (totalData (int)));
}

void Core::handleFinish (bool result)
{
    delete Thread_;
}

