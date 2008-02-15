#include "remoter.h"

void Remoter::Init ()
{
    IsShown_ = false;
    Ui_.setupUi (this);
}

void Remoter::Release ()
{
}

QString Remoter::GetName () const
{
    return tr ("Remoter");
}

QString Remoter::GetInfo () const
{
    return tr ("Server providing remote access to other plugins."); 
}

QString Remoter::GetStatusbarMessage () const
{
    return QString ();
}

IInfo& Remoter::SetID (long unsigned int id)
{
    ID_ = id;
    return *this;
}

unsigned long int Remoter::GetID () const
{
    return ID_;
}

QStringList Remoter::Provides () const
{
    return QStringList ("remoteaccess");
}

QStringList Remoter::Needs () const
{
    return QStringList ();
}

QStringList Remoter::Uses () const
{
    return QStringList ("remoteable");
}

void Remoter::SetProvider (QObject *provider, const QString& feature)
{
    Q_UNUSED (feature);
    if (feature == "remoteable")
        Objects_ << provider;
}

void Remoter::PushMainWindowExternals (const MainWindowExternals&)
{
}

QIcon Remoter::GetIcon () const
{
    return windowIcon ();
}

void Remoter::SetParent (QWidget *parent)
{
    setParent (parent);
}

void Remoter::ShowWindow ()
{
    IsShown_ = 1 - IsShown_;
    IsShown_ ? show () : hide ();
}

void Remoter::ShowBalloonTip ()
{
}

void Remoter::closeEvent (QCloseEvent*)
{
    IsShown_ = false;
}

Q_EXPORT_PLUGIN2 (leechcraft_remoter, Remoter);

