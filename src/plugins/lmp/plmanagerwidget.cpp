/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "plmanagerwidget.h"
#include <QMenu>
#include <QMessageBox>
#include "core.h"
#include "playlistmanager.h"
#include "player.h"

namespace LC
{
namespace LMP
{
	PLManagerWidget::PLManagerWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		auto mgr = Core::Instance ().GetPlaylistManager ();
		Ui_.PlaylistsTree_->setModel (mgr->GetPlaylistsModel ());
		Ui_.PlaylistsTree_->expandAll ();

		connect (Ui_.PlaylistsTree_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handlePlaylistSelected (QModelIndex)));

		DeletePlaylistAction_ = new QAction (tr ("Delete playlist"), Ui_.PlaylistsTree_);
		DeletePlaylistAction_->setProperty ("ActionIcon", "list-remove");
		DeletePlaylistAction_->setShortcut (Qt::Key_Delete);
		DeletePlaylistAction_->setShortcutContext (Qt::WidgetShortcut);
		connect (DeletePlaylistAction_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDeleteSelected ()));
		Ui_.PlaylistsTree_->addAction (DeletePlaylistAction_);
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

		const auto mgr = Core::Instance ().GetPlaylistManager ();
		if (!mgr->CanDeletePlaylist (idx))
			return;

		if (QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to delete playlist %1?")
					.arg ("<em>" + idx.data ().toString () + "</em>"),
				QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		mgr->DeletePlaylist (Ui_.PlaylistsTree_->currentIndex ());
	}

	void PLManagerWidget::handlePlaylistSelected (const QModelIndex& index)
	{
		auto mgr = Core::Instance ().GetPlaylistManager ();
		const auto& sources = mgr->GetSources (index);
		if (sources.isEmpty ())
			return;

		Player_->clear ();
		Player_->SetNativePlaylist (sources);
	}
}
}
