/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "ktorrentimportpage.h"
#include <QDomDocument>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QUrl>
#include <QMap>
#include <util/xpc/util.h>

namespace LeechCraft
{
namespace NewLife
{
namespace Importers
{
	KTorrentImportPage::KTorrentImportPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: EntityGeneratingPage { proxy, parent }
	{
		Ui_.setupUi (this);
		Ui_.ImportSettings_->setText (Ui_.ImportSettings_->text ().arg ("KTorrent"));

		setTitle (tr ("KTorrent's torrents import"));
		setSubTitle (tr ("Select KTorrent's torrents"));
	}

	bool KTorrentImportPage::CheckValidity (const QString& filename) const
	{
		QFile file (filename);
		if (!file.exists () ||
				!file.open (QIODevice::ReadOnly))
			return false;

		return true;
	}

	bool KTorrentImportPage::isComplete () const
	{
		return CheckValidity (Ui_.FileLocation_->text ());
	}

	int KTorrentImportPage::nextId () const
	{
		return -1;
	}

	void KTorrentImportPage::initializePage ()
	{
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));

		QString defaultFile = QDir::homePath () + "/.kde/share/config/ktorrentrc";
		if (CheckValidity (defaultFile))
			Ui_.FileLocation_->setText (defaultFile);
	}

	void KTorrentImportPage::on_Browse__released ()
	{
		QString filename = QFileDialog::getOpenFileName (this,
				tr ("Select KTorrent's configuration file"),
				QDir::homePath (),
				"All files (*.*)");
		if (filename.isEmpty ())
			return;

		if (!CheckValidity (filename))
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Can't read KTorrent's configuration file"));
			return;
		}

		Ui_.FileLocation_->setText (filename);
	}

	bool KTorrentImportPage::GetTorrentSettings (const QString& path,
			QMap<QString, QVariant>& settings) const
	{
		QDir torrentDir (path);
		if (!torrentDir.exists () ||
				!torrentDir.isReadable())
			return false;

		QFileInfoList files = torrentDir
			.entryInfoList (QDir::Files | QDir::Readable, QDir::Unsorted);

		for (int i = 0; i < files.size (); ++i)
		{
			QFile file (files.at (i).fileName ());
			settings.insert (files.at (i).fileName (), file.readAll ());
		}
		return true;
	}

	void KTorrentImportPage::handleAccepted ()
	{
		QString filename = Ui_.FileLocation_->text ();
		if (!CheckValidity (filename))
			return;

		Entity e = Util::MakeEntity (QUrl::fromLocalFile (filename),
				QString (),
				FromUserInitiated,
				"x-leechcraft/bittorrent-import");

		if (Ui_.ImportSettings_->checkState () == Qt::Checked)
		{
			QSettings settings (filename, QSettings::IniFormat);
			if (settings.status () == QSettings::NoError)
			{
				QMap<QString, QVariant> additional;
				settings.beginGroup ("downloads");
				if (settings.contains ("completedDir"))
					additional ["CompletedDir"] = settings.value ("completedDir");
				if (settings.contains ("dhtPort"))
					additional ["DHTPort"] = settings.value ("dhtPort");
				if (settings.contains ("dhtSupport"))
					additional ["DHTSupport"] = settings.value ("dhtSupport");
				if (settings.contains ("lastSaveDir"))
					additional ["LastSaveDir"] = settings.value ("lastSaveDir");
				if (settings.contains ("oldTorrentsImported"))
					additional ["OldTorrentsImported"] = settings.value ("oldTorrentsImported");
				if (settings.contains ("saveDir"))
					additional ["SaveDir"] = settings.value ("saveDir");
				if (settings.contains ("TempDir"))
				{
					additional ["TempDir"] = settings.value ("TempDir");
					QDir tempDir (settings.value ("TempDir").toString ());
					if (tempDir.exists () &&
							tempDir.isReadable ())
					{
						QFileInfoList torrentsDir = tempDir.entryInfoList (QStringList ("tor"),
								QDir::Dirs | QDir::Readable,
								QDir::Unsorted);
						QList<QVariant> list;
						for (int i = 0; i < torrentsDir.size (); ++i)
						{
							QMap<QString, QVariant> map;
							GetTorrentSettings (torrentsDir.at (i).absoluteFilePath (),
									map);
							list << map;
						}
						additional ["BitTorrentImportTorrents"] = list;
					}
				}
				else
				{
					additional ["TempDir"] = "~/.kde/share/apps/ktorrent";
					// TODO later
				}
				if (settings.contains ("TorrentCopyDir"))
					additional ["TorrentCopyDir"] = settings.value ("torrentCopyDir");
				settings.endGroup ();

				e.Additional_ ["BitTorrent/SettingsImportData"] = additional;
				e.Additional_ ["UserVisibleName"] = tr ("KTorrent settings");
			}
			else
				QMessageBox::critical (this,
						"LeechCraft",
						tr ("Could not access or parse KTorrent settings."));
		}
		SendEntity (e);
	}
}
}
}
