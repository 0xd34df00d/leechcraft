#include "lmp.h"
#include <QToolBar>
#include <seekslider.h>
#include <volumeslider.h>
#include <plugininterface/util.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "entitychecker.h"

using namespace LeechCraft::Plugins::LMP;

void LMP::Init (ICoreProxy_ptr)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("lmp"));
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
	return QIcon (":/resources/images/lmp.png");
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
}

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LeechCraft::Plugins::LMP::LMP);

