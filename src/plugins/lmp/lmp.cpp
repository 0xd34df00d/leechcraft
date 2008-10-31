#include "lmp.h"
#include "core.h"
#include "tabwidget.h"

void LMP::Init ()
{
	TabWidget_ = new TabWidget ();
	Core::Instance ()
		.Reinitialize ("/home/d34df00d/music/Allele/2005 Point of Origin/01 - Allele - Fake.mp3");

	Phonon::createPath (Core::Instance ().GetMediaObject (),
			TabWidget_->GetVideoOutput ());
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
	return TabWidget_;
}

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LMP);

