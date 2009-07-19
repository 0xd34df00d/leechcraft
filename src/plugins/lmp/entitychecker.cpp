#include "entitychecker.h"
#include <QApplication>
#include <QTextCodec>
#include <QFileInfo>
#include <QUrl>
#include <interfaces/structures.h>
#include "phonon.h"
#include "xmlsettingsmanager.h"

using namespace LeechCraft;
using namespace LeechCraft::Plugins::LMP;

EntityChecker::EntityChecker (const LeechCraft::DownloadEntity& e)
: Result_ (false)
, Break_ (false)
{
	QString source;
	if (e.Entity_.canConvert<QUrl> ())
		source = e.Entity_.toUrl ().toLocalFile ();
	else if (e.Entity_.canConvert<QString> ())
		source = e.Entity_.toString ();
	else
		return;

	QFileInfo fi (source);
	if (!fi.exists ())
		return;

	if (XmlSettingsManager::Instance ()->property ("TestOnly").toBool () &&
			!XmlSettingsManager::Instance ()->
				property ("TestExtensions").toString ()
				.split (' ', QString::SkipEmptyParts).contains (fi.suffix ()))
		return;

	std::auto_ptr<Phonon::MediaObject> mo (new Phonon::MediaObject ());
	std::auto_ptr<Phonon::AudioOutput> ao (new Phonon::AudioOutput ());
	ao->setMuted (true);
	Phonon::createPath (mo.get (), ao.get ());
	mo->setCurrentSource (source);
	connect (mo.get (),
			SIGNAL (stateChanged (Phonon::State, Phonon::State)),
			this,
			SLOT (stateChanged (Phonon::State)));
	mo->play ();

	while (!Break_)
		qApp->processEvents ();
}

bool EntityChecker::Can () const
{
	return Result_;
}

void EntityChecker::stateChanged (Phonon::State st)
{
	switch (st)
	{
		case Phonon::PlayingState:
			Result_ = true;
		case Phonon::ErrorState:
			Break_ = true;
		default:
			break;
	}
}

