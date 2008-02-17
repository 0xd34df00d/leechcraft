#include <QtCore/QtCore>
#include "cron.h"
#include "core.h"
#include "globals.h"

void Cron::Init ()
{
    Core_ = new Core (this);
    connect (Core_, SIGNAL (shot (quint64)), this, SIGNAL (shot (quint64)));
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
    Providers_ [feature] = provider;
}

void Cron::PushMainWindowExternals (const MainWindowExternals&)
{
}

void Cron::Release ()
{
    Core_->Release ();
    delete Core_;
}

void Cron::addSingleShot (QDateTime dt)
{
    emit added (Core_->AddSingleShot (dt));
}

Q_EXPORT_PLUGIN2 (leechcraft_cron, Cron);

