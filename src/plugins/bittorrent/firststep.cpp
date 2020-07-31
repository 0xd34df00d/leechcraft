/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include <QFileDialog>
#include <QDir>
#include "firststep.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace BitTorrent
{
	FirstStep::FirstStep (QWidget *parent)
	: QWizardPage (parent)
	{
		setupUi (this);
		registerField ("Output", Output_);
		registerField ("AnnounceURL*", AnnounceURL_);
		registerField ("Date", Date_);
		registerField ("Comment", Comment_);
		registerField ("RootPath", RootPath_);
		Date_->setDateTime (QDateTime::currentDateTime ());
		Output_->setText (XmlSettingsManager::Instance ()->
				property ("LastMakeTorrentDirectory").toString ());
		RootPath_->setText (XmlSettingsManager::Instance ()->
				property ("LastAddDirectory").toString ());
		connect (RootPath_,
				SIGNAL (textChanged (const QString&)),
				this,
				SIGNAL (completeChanged ()));
	}

	bool FirstStep::isComplete () const
	{
		QFileInfo info (RootPath_->text ());
		return info.exists () &&
			info.isReadable ();
	}

	QString FirstStep::PrepareDirectory () const
	{
		QString directory = RootPath_->text ();
		if (!QFileInfo (directory).isDir ())
			directory = QFileInfo (directory).absolutePath ();

		if (!QFileInfo (directory).exists ())
			directory = QDir::homePath ();

		if (!directory.endsWith ('/'))
			directory.append ('/');

		return directory;
	}

	void FirstStep::on_BrowseOutput__released ()
	{
		QString last = XmlSettingsManager::Instance ()->
			property ("LastMakeTorrentDirectory").toString ();
		if (!last.endsWith ('/'))
			last += '/';
		if (!QFileInfo (last).exists ())
			last = QDir::homePath ();

		QString directory = QFileDialog::getSaveFileName (this,
				tr ("Select where to save torrent file"),
				last);
		if (directory.isEmpty ())
			return;

		Output_->setText (directory);
		XmlSettingsManager::Instance ()->
			setProperty ("LastMakeTorrentDirectory",
					QFileInfo (directory).absolutePath ());
	}

	void FirstStep::on_BrowseFile__released ()
	{
		QString path = QFileDialog::getOpenFileName (this,
				tr ("Select torrent contents"),
				PrepareDirectory ());
		if (path.isEmpty ())
			return;

		RootPath_->setText (path);
		XmlSettingsManager::Instance ()->
			setProperty ("LastAddDirectory",
					QFileInfo (path).absolutePath ());

		emit completeChanged ();
	}

	void FirstStep::on_BrowseDirectory__released ()
	{
		QString path = QFileDialog::getExistingDirectory (this,
				tr ("Select torrent contents"),
				PrepareDirectory ());
		if (path.isEmpty ())
			return;

		RootPath_->setText (path);
		XmlSettingsManager::Instance ()->
			setProperty ("LastAddDirectory",
					path);

		emit completeChanged ();
	}
}
}
