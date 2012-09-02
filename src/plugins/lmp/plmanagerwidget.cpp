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
	{
		Ui_.setupUi (this);

		auto mgr = Core::Instance ().GetPlaylistManager ();
		Ui_.PlaylistsTree_->setModel (mgr->GetPlaylistsModel ());
		Ui_.PlaylistsTree_->expandAll ();

		connect (Ui_.PlaylistsTree_,
				SIGNAL (doubleClicked (QModelIndex)),
				this,
				SLOT (handlePlaylistSelected (QModelIndex)));
	}

	void PLManagerWidget::SetPlayer (Player *player)
	{
		Player_ = player;
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
