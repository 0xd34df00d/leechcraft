/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QModelIndexList>
#include <QCoreApplication>

class QAction;
class QMenu;
class QToolBar;
class QWidget;

namespace libtorrent
{
	class session;
}

namespace LC::BitTorrent
{
	class ListActions final : public QObject
	{
		Q_DECLARE_TR_FUNCTIONS (LC::BitTorrent::ListActions)
	public:
		struct Dependencies
		{
			libtorrent::session& Session_;
			std::function<QWidget* ()> GetPreferredParent_;
		};
	private:
		const Dependencies D_;

		QToolBar * const Toolbar_;

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

		QModelIndex CurIdx_;
		QModelIndexList CurSelection_;
	public:
		explicit ListActions (const Dependencies&, QWidget* = nullptr);

		QToolBar* GetToolbar () const;

		void SetCurrentIndex (const QModelIndex&);
		void SetCurrentSelection (const QModelIndexList&);

		QMenu* MakeContextMenu () const;
	private:
		void SetActionsEnabled ();
		QList<int> GetSelectedHandlesIndices () const;
	};
}
