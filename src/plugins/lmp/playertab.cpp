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
#include "player.h"

namespace LeechCraft
{
namespace LMP
{
	PlayerTab::PlayerTab (const TabClassInfo& info, QObject *plugin, QWidget *parent)
	: QWidget (parent)
	, Plugin_ (plugin)
	, TC_ (info)
	, Player_ (new Player (this))
	, PlaylistToolbar_ (new QToolBar ())
	{
		Ui_.setupUi (this);

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
		return 0;
	}

	void PlayerTab::SetupPlaylist ()
	{
		Ui_.Playlist_->setModel (Player_->GetPlaylistModel ());

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
