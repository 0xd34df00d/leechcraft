#include "lmp.h"
#include <QToolBar>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "entitychecker.h"

using namespace LeechCraft::Plugins::LMP;
using namespace LeechCraft::Util;

void LMP::Init (ICoreProxy_ptr proxy)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("lmp"));

	SettingsDialog_.reset (new XmlSettingsDialog ());
	SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			"lmpsettings.xml");

	Core::Instance ().SetCoreProxy (proxy);

	connect (&Core::Instance (),
			SIGNAL (bringToFront ()),
			this,
			SIGNAL (bringToFront ()));
}

void LMP::Release ()
{
	Core::Instance ().Release ();
	XmlSettingsManager::Instance ()->Release ();
}

QString LMP::GetName () const
{
	return "LMP";
}

QString LMP::GetInfo () const
{
	return "LeechCraft Media Player";
}

QStringList LMP::Provides () const
{
	return QStringList ("media");
}

QStringList LMP::Needs () const
{
	return QStringList ();
}

QStringList LMP::Uses () const
{
	return QStringList ();
}

void LMP::SetProvider (QObject*, const QString&)
{
}

QIcon LMP::GetIcon () const
{
	return QIcon (":/plugins/lmp/resources/images/lmp.svg");
}

boost::shared_ptr<XmlSettingsDialog> LMP::GetSettingsDialog () const
{
	return SettingsDialog_;
}

bool LMP::CouldHandle (const LeechCraft::DownloadEntity& e) const
{
	EntityChecker ec (e);
	return ec.Can ();
}

void LMP::Handle (LeechCraft::DownloadEntity e)
{
	if (!CouldHandle (e))
		return;

	Core::Instance ().Handle (e);
}

QList<QAction*> LMP::GetActions () const
{
	QList<QAction*> result;
	result += Core::Instance ().GetShowAction ();
	return result;
}

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LeechCraft::Plugins::LMP::LMP);

