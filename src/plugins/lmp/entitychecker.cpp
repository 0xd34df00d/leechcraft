/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

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
	struct MimeChecker
	{
		bool operator () (const QString& mime)
		{
			if (mime == "application/ogg")
				return true;
			if (mime.startsWith ("audio/"))
				return true;
			if (mime.startsWith ("video/"))
				return true;
			return false;
		}
	};

	if (e.Entity_.canConvert<QNetworkReply*> () &&
			MimeChecker () (e.Mime_))
	{
		Result_ = true;
		return;
	}
	if (e.Entity_.canConvert<QUrl> ())
	{
		QUrl url = e.Entity_.toUrl ();
		QString extension = QFileInfo (url.toLocalFile ()).suffix ();

		QStringList goodExt = XmlSettingsManager::Instance ()->
			property ("TestExtensions").toString ()
			.split (' ', QString::SkipEmptyParts);

		Result_ = goodExt.contains (extension);

		return;
	}

	Result_ = false;

	/* TODO
	 * Use this code path when we will be able to figure out how to
	 * synchronously check a local file if it's playable.
	QString source;
	if (e.Entity_.canConvert<QUrl> ())
	{
		QUrl url = e.Entity_.toUrl ();
		if (url.scheme () == "file")
			source = url.toLocalFile ();
		else
		{
			QString extension = QFileInfo (url.path ()).suffix ();

			QStringList goodExt = XmlSettingsManager::Instance ()->
				property ("TestExtensions").toString ()
				.split (' ', QString::SkipEmptyParts);

			Result_ = goodExt.contains (extension);
			return;
		}
	}
	else if (e.Entity_.canConvert<QString> ())
		source = e.Entity_.toString ();
	else if (e.Additional_ ["SourceURL"].canConvert<QUrl> ())
	{
		QUrl url = e.Additional_ ["SourceURL"].toUrl ();
		QString extension = QFileInfo (url.path ()).suffix ();

		QStringList goodExt = XmlSettingsManager::Instance ()->
			property ("TestExtensions").toString ()
			.split (' ', QString::SkipEmptyParts);

		Result_ = goodExt.contains (extension);
	}
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
		*/
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

