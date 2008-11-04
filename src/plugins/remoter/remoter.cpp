#include <QTranslator>
#include "remoter.h"
#include "core.h"

void Remoter::Init ()
{
    Q_INIT_RESOURCE (resources);
    QTranslator *transl = new QTranslator (this);
    QString localeName = QString(::getenv ("LANG")).left (2);
    if (localeName.isNull () || localeName.isEmpty ())
        localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_remoter_") + localeName);
    qApp->installTranslator (transl);

    IsShown_ = false;
    Ui_.setupUi (this);
}

void Remoter::Release ()
{
    Core::Instance ().Release ();
}

QString Remoter::GetName () const
{
    return tr ("Remoter");
}

QString Remoter::GetInfo () const
{
    return tr ("Server providing remote access to other plugins."); 
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
    Core::Instance ().AddObject (provider, feature);
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

void Remoter::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void Remoter::handleHidePlugins ()
{
    IsShown_ = false;
    hide ();
}

Q_EXPORT_PLUGIN2 (leechcraft_remoter, Remoter);

