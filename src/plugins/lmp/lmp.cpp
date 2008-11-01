#include "lmp.h"
#include <plugininterface/util.h>
#include "core.h"

void LMP::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("lmp"));
	Ui_.setupUi (this);

	connect (&Core::Instance (),
			SIGNAL (stateUpdated (const QString&)),
			this,
			SLOT (handleStateUpdated (const QString&)));

	Core::Instance ()
		.Reinitialize ("/home/d34df00d/2008.avi");
//		.Reinitialize ("/home/d34df00d/music/Allele/2005 Point of Origin/01 - Allele - Fake.mp3");

	Phonon::createPath (Core::Instance ().GetMediaObject (),
			Ui_.VideoWidget_);
	Phonon::createPath (Core::Instance ().GetMediaObject (),
			Core::Instance ().GetAudioOutput ());

	Core::Instance ().GetMediaObject ()->play ();
}

void LMP::Release ()
{
	Core::Instance ().Release ();
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
	return QIcon ();
}

QWidget* LMP::GetTabContents ()
{
	return this;
}

void LMP::handleStateUpdated (const QString& state)
{
	Ui_.State_->setText (state);
}

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LMP);

