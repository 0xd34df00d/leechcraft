#include <QDateTime>
#include <QtDebug>
#include <QTimer>
#include <plugininterface/proxy.h>
#include "core.h"
#include "globals.h"

Core::Core (QObject *parent)
: QObject (parent)
, FreeIDs_ (10000)
{
    qsrand (QDateTime::currentDateTime ().toTime_t ());
    for (int i = 0; i < FreeIDs_.size (); ++i)
        FreeIDs_ [i] = i;
    ReadSettings ();
}

void Core::Release ()
{
    writeSettings ();
}

int Core::AddSingleshot (const QDateTime& dt)
{
    int id = FreeIDs_.last ();
    FreeIDs_.pop_back ();

    QTimer *timer = new QTimer (this);
    connect (timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
    Singleshots_ [timer] = id;
    timer->setSingleShot (true);
    timer->start (QDateTime::currentDateTime ().secsTo (dt) * 1000);
    return id;
}

int Core::AddInterval (int msecs)
{
    int id = FreeIDs_.last ();
    FreeIDs_.pop_back ();

    QTimer *timer = new QTimer (this);
    connect (timer, SIGNAL (timeout ()), this, SLOT (timeout ()));
    Intervals_ [timer] = id;
    timer->start (msecs);
    return id;
}

void Core::ReadSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (Globals::Name);
    settings.endGroup ();
}

void Core::writeSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName ());
    settings.beginGroup (Globals::Name);
    settings.endGroup ();
}

void Core::timeout ()
{
    QTimer *timer = qobject_cast<QTimer*> (sender ());
    if (!timer)
        return;
    if (Singleshots_.contains (timer))
    {
        int id = Singleshots_ [timer];
        emit shot (id);
        FreeIDs_.push_back (id);
        delete timer;
    }
    else if (Intervals_.contains (timer))
        emit shot (Intervals_ [timer]);
    else
        qWarning () << Q_FUNC_INFO << timer << "not found in any of internal data structures";
}

