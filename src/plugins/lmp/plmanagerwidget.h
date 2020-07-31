/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_plmanagerwidget.h"

namespace LC
{
namespace LMP
{
	class Player;

	class PLManagerWidget : public QWidget
	{
		Q_OBJECT

		Ui::PLManagerWidget Ui_;
		Player *Player_ = nullptr;

		QAction *DeletePlaylistAction_;
	public:
		PLManagerWidget (QWidget* = 0);

		void SetPlayer (Player*);
	private slots:
		void on_PlaylistsTree__customContextMenuRequested (const QPoint&);
		void handleDeleteSelected ();
		void handlePlaylistSelected (const QModelIndex&);
	};
}
}
