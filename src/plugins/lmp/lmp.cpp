#include "lmp.h"
#include <QToolBar>
#include <Phonon>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "entitychecker.h"

using namespace LeechCraft::Plugins::LMP;
using namespace LeechCraft::Util;

void LMP::Init (ICoreProxy_ptr)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("lmp"));

	SettingsDialog_.reset (new XmlSettingsDialog ());
	SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			":/plugins/lmp/lmpsettings.xml");

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
	return QIcon (":/plugins/lmp/resources/images/lmp.png");
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

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LeechCraft::Plugins::LMP::LMP);

