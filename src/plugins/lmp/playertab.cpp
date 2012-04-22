/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "playertab.h"
#include <QToolBar>
#include <QFileDialog>
#include <QFileSystemModel>
#include <phonon/seekslider.h>
#include "player.h"
#include "playlistdelegate.h"

namespace LeechCraft
{
namespace LMP
{
	PlayerTab::PlayerTab (const TabClassInfo& info, QObject *plugin, QWidget *parent)
	: QWidget (parent)
	, Plugin_ (plugin)
	, TC_ (info)
	, FSModel_ (new QFileSystemModel (this))
	, Player_ (new Player (this))
	, PlaylistToolbar_ (new QToolBar ())
	, TabToolbar_ (new QToolBar ())
	{
		Ui_.setupUi (this);

		SetupToolbar ();
		SetupFSBrowser ();
		SetupPlaylist ();
	}

	TabClassInfo PlayerTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* PlayerTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void PlayerTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* PlayerTab::GetToolBar () const
	{
		return TabToolbar_;
	}

	void PlayerTab::SetupToolbar ()
	{
		QAction *previous = new QAction (tr ("Previous track"), this);
		previous->setProperty ("ActionIcon", "media-skip-backward");
		connect (previous,
				SIGNAL (triggered ()),
				Player_,
				SLOT (previousTrack ()));
		TabToolbar_->addAction (previous);

		QAction *pause= new QAction (tr ("Play/pause"), this);
		pause->setProperty ("ActionIcon", "media-playback-pause");
		connect (pause,
				SIGNAL (triggered ()),
				Player_,
				SLOT (togglePause ()));
		TabToolbar_->addAction (pause);

		QAction *stop = new QAction (tr ("Stop"), this);
		stop->setProperty ("ActionIcon", "media-playback-stop");
		connect (stop,
				SIGNAL (triggered ()),
				Player_,
				SLOT (stop ()));
		TabToolbar_->addAction (stop);

		QAction *next = new QAction (tr ("Next track"), this);
		next->setProperty ("ActionIcon", "media-skip-forward");
		connect (next,
				SIGNAL (triggered ()),
				Player_,
				SLOT (nextTrack ()));
		TabToolbar_->addAction (next);

		auto seekSlider = new Phonon::SeekSlider (Player_->GetSourceObject ());
		TabToolbar_->addWidget (seekSlider);
	}

	void PlayerTab::SetupFSBrowser ()
	{
		FSModel_->setReadOnly (true);
		FSModel_->setRootPath (QDir::rootPath ());
		Ui_.FSTree_->setModel (FSModel_);

		QAction *addToPlaylist = new QAction (tr ("Add to playlist"), this);
		addToPlaylist->setProperty ("ActionIcon", "list-add");
		connect (addToPlaylist,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromFSBrowser ()));
		Ui_.FSTree_->addAction (addToPlaylist);
	}

	void PlayerTab::SetupPlaylist ()
	{
		Ui_.Playlist_->setItemDelegate (new PlaylistDelegate (Ui_.Playlist_));
		Ui_.Playlist_->setModel (Player_->GetPlaylistModel ());
		connect (Ui_.Playlist_,
				SIGNAL (doubleClicked (QModelIndex)),
				Player_,
				SLOT (play (QModelIndex)));
		connect (Player_,
				SIGNAL (insertedAlbum (QModelIndex)),
				Ui_.Playlist_,
				SLOT (expand (QModelIndex)));

		Ui_.PlaylistLayout_->addWidget (PlaylistToolbar_);

		QAction *clearPlaylist = new QAction (tr ("Clear..."), this);
		clearPlaylist->setProperty ("ActionIcon", "edit-clear-list");
		connect (clearPlaylist,
				SIGNAL (triggered ()),
				Player_,
				SLOT (clear ()));
		PlaylistToolbar_->addAction (clearPlaylist);

		QAction *loadFiles = new QAction (tr ("Load from disk..."), this);
		loadFiles->setProperty ("ActionIcon", "document-open");
		connect (loadFiles,
				SIGNAL (triggered ()),
				this,
				SLOT (loadFromDisk ()));
		PlaylistToolbar_->addAction (loadFiles);
	}

	namespace
	{
		QStringList RecIterate (const QString& dirPath)
		{
			QStringList result;
			QStringList nameFilters;
			nameFilters << ".ogg"
					<< ".flac"
					<< ".mp3"
					<< ".wav";
			QDirIterator iterator (dirPath, QDirIterator::Subdirectories);
			while (iterator.hasNext ())
			{
				const QString& path = iterator.next ();
				Q_FOREACH (const QString& name, nameFilters)
					if (path.endsWith (name, Qt::CaseInsensitive))
					{
						result << path;
						break;
					}
			}
			return result;
		}
	}

	void PlayerTab::loadFromFSBrowser ()
	{
		const QModelIndex& index = Ui_.FSTree_->currentIndex ();
		if (!index.isValid ())
			return;

		const QFileInfo& fi = FSModel_->fileInfo (index);

		QList<Phonon::MediaSource> queue;
		if (fi.isDir ())
		{
			const auto& paths = RecIterate (fi.absoluteFilePath ());
			std::transform (paths.begin (), paths.end (), std::back_inserter (queue),
					[] (const QString& path) { return Phonon::MediaSource (path); });
		}
		else
			queue << fi.absoluteFilePath ();
		Player_->Enqueue (queue);
	}

	void PlayerTab::loadFromDisk ()
	{
		QStringList files = QFileDialog::getOpenFileNames (this,
				tr ("Load files"),
				QDir::homePath (),
				tr ("Music files (*.ogg *.flac *.mp3 *.wav);;All files (*.*)"));
		QList<Phonon::MediaSource> queue;
		Q_FOREACH (const QString& file, files)
			queue << file;
		Player_->Enqueue (queue);
	}
}
}
