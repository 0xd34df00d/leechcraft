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

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_torrenttab.h"

namespace LeechCraft
{
namespace Plugins
{
namespace BitTorrent
{
	class TorrentTab : public QWidget
					 , public ITabWidget
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget)

		Ui::TorrentTab Ui_;

		const TabClassInfo TC_;
		QObject *ParentMT_;

		QToolBar *Toolbar_;
		QAction *OpenTorrent_;
		QAction *RemoveTorrent_;
		QAction *Resume_;
		QAction *Stop_;
		QAction *CreateTorrent_;
		QAction *MoveUp_;
		QAction *MoveDown_;
		QAction *MoveToTop_;
		QAction *MoveToBottom_;
		QAction *ForceReannounce_;
		QAction *ForceRecheck_;
		QAction *OpenMultipleTorrents_;
		QAction *IPFilter_;
		QAction *MoveFiles_;
		QAction *ChangeTrackers_;
		QAction *MakeMagnetLink_;
	public:
		TorrentTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		QToolBar* GetToolBar () const;
		void Remove ();
	private:
		int GetCurrentTorrent () const;
		QList<int> GetSelectedRows () const;
		QModelIndexList GetSelectedRowIndexes () const;
	private slots:
		void handleTorrentSelected (const QModelIndex&);
		void setActionsEnabled ();

		void on_OpenTorrent__triggered ();
		void on_OpenMultipleTorrents__triggered ();
		void on_IPFilter__triggered ();
		void on_CreateTorrent__triggered ();
		void on_RemoveTorrent__triggered ();
		void on_Resume__triggered ();
		void on_Stop__triggered ();
		void on_MoveUp__triggered ();
		void on_MoveDown__triggered ();
		void on_MoveToTop__triggered ();
		void on_MoveToBottom__triggered ();
		void on_ForceReannounce__triggered ();
		void on_ForceRecheck__triggered ();
		void on_ChangeTrackers__triggered ();
		void on_MoveFiles__triggered ();
		void on_MakeMagnetLink__triggered ();
	signals:
		void removeTab (QWidget*);
	};
}
}
}
