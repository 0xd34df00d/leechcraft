#include <QDateTime>
#include "cron.h"
#include "core.h"
#include "globals.h"

void Cron::Init ()
{
    Core_ = new Core (this);
    connect (Core_, SIGNAL (shot (int)), this, SIGNAL (shot (int)));
}

QString Cron::GetName () const
{
    return Globals::Name;
}

QString Cron::GetInfo () const
{
    return tr ("Provides simple scheduling policies.");
}

QString Cron::GetStatusbarMessage () const
{
    return "";
}

IInfo& Cron::SetID (IInfo::ID_t id)
{
    ID_ = id;
    return *this;
}

IInfo::ID_t Cron::GetID () const
{
    return ID_;
}

QStringList Cron::Provides () const
{
    return QStringList ("cron");
}

QStringList Cron::Needs () const
{
    return QStringList ();
}

QStringList Cron::Uses () const
{
    return QStringList ();
}

void Cron::SetProvider (QObject *provider, const QString& feature)
{
}

void Cron::PushMainWindowExternals (const MainWindowExternals&)
{
}

void Cron::Release ()
{
    Core_->Release ();
    delete Core_;
}

void Cron::addSingleshot (const QDateTime& dt, int *id)
{
    *id =Core_->AddSingleshot (dt);
}

void Cron::addInterval (int msecs, int *id)
{
    *id =Core_->AddInterval (msecs);
}

Q_EXPORT_PLUGIN2 (leechcraft_cron, Cron);

