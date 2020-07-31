/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include "ui_torrenttab.h"

namespace LC
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
		QAction *AddMagnet_;
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

		QSortFilterProxyModel *ViewFilter_;
	public:
		TorrentTab (const TabClassInfo&, QObject*);

		TabClassInfo GetTabClassInfo () const;
		QObject* ParentMultiTabs ();
		QToolBar* GetToolBar () const;
		void Remove ();

		void SetCurrentTorrent (int);
	private:
		int GetCurrentTorrent () const;
		QList<int> GetSelectedRows () const;
		QModelIndexList GetSelectedRowIndexes () const;
	private slots:
		void setActionsEnabled ();

		void on_TorrentsView__customContextMenuRequested (const QPoint&);

		void handleOpenTorrentTriggered ();
		void handleOpenTorrentAccepted ();
		void handleAddMagnetTriggered ();
		void handleOpenMultipleTorrentsTriggered ();

		void handleIPFilterTriggered ();
		void handleCreateTorrentTriggered ();
		void handleRemoveTorrentTriggered ();
		void handleResumeTriggered ();
		void handleStopTriggered ();
		void handleMoveUpTriggered ();
		void handleMoveDownTriggered ();
		void handleMoveToTopTriggered ();
		void handleMoveToBottomTriggered ();
		void handleForceReannounceTriggered ();
		void handleForceRecheckTriggered ();
		void handleChangeTrackersTriggered ();
		void handleMoveFilesTriggered ();
		void handleMakeMagnetLinkTriggered ();
	signals:
		void removeTab (QWidget*);
	};
}
}
