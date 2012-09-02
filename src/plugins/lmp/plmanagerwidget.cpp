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

#include "plmanagerwidget.h"
#include <QMenu>
#include <QMessageBox>
#include "core.h"
#include "playlistmanager.h"
#include "player.h"

namespace LeechCraft
{
namespace LMP
{
	PLManagerWidget::PLManagerWidget (QWidget *parent)
	: QWidget (parent)
	, Player_ (0)
	, DeletePlaylistAction_ (new QAction (tr ("Delete playlist"), this))
	{
		Ui_.setupUi (this);

		auto mgr = Core::Instance ().GetPlaylistManager ();
		Ui_.PlaylistsTree_->setModel (mgr->GetPlaylistsModel ());
		Ui_.PlaylistsTree_->expandAll ();

		connect (Ui_.PlaylistsTree_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handlePlaylistSelected (QModelIndex)));

		DeletePlaylistAction_->setProperty ("ActionIcon", "list-remove");
		connect (DeletePlaylistAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDeleteSelected ()));
	}

	void PLManagerWidget::SetPlayer (Player *player)
	{
		Player_ = player;
	}

	void PLManagerWidget::on_PlaylistsTree__customContextMenuRequested (const QPoint& pos)
	{
		const auto& idx = Ui_.PlaylistsTree_->indexAt (pos);
		if (!idx.isValid ())
			return;

		auto mgr = Core::Instance ().GetPlaylistManager ();
		if (!mgr->CanDeletePlaylist (idx))
			return;

		auto menu = new QMenu (Ui_.PlaylistsTree_);
		menu->addAction (DeletePlaylistAction_);
		menu->setAttribute (Qt::WA_DeleteOnClose);
		menu->exec (Ui_.PlaylistsTree_->viewport ()->mapToGlobal (pos));
	}

	void PLManagerWidget::handleDeleteSelected ()
	{
		const auto& idx = Ui_.PlaylistsTree_->currentIndex ();
		if (QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to delete playlist %1?")
					.arg ("<em>" + idx.data ().toString () + "</em>"),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		auto mgr = Core::Instance ().GetPlaylistManager ();
		mgr->DeletePlaylist (Ui_.PlaylistsTree_->currentIndex ());
	}

	void PLManagerWidget::handlePlaylistSelected (const QModelIndex& index)
	{
		auto mgr = Core::Instance ().GetPlaylistManager ();
		const auto& sources = mgr->GetSources (index);
		if (sources.isEmpty ())
			return;

		Player_->Enqueue (sources, false);
	}
}
}
