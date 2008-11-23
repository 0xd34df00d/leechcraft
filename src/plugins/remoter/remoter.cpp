#include "remoter.h"
#include <QTranslator>
#include <plugininterface/mergemodel.h>
#include <plugininterface/util.h>
#include "core.h"

void Remoter::Init ()
{
	LeechCraft::Util::InstallTranslator ("remoter");

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
    return QStringList ("*")
		<< "services::historyModel"
		<< "services::downloadersModel";
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

void Remoter::pushHistoryModel (MergeModel *model) const
{
	Core::Instance ().SetHistoryModel (model);
}

void Remoter::pushDownloadersModel (MergeModel *model) const
{
	Core::Instance ().SetDownloadersModel (model);
}

Q_EXPORT_PLUGIN2 (leechcraft_remoter, Remoter);

