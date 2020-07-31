/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_fsbrowserwidget.h"

namespace LC
{
namespace LMP
{
	class Player;
	class FSModel;

	class FSBrowserWidget : public QWidget
	{
		Q_OBJECT

		Ui::FSBrowserWidget Ui_;

		Player *Player_;
		FSModel *FSModel_;
		QAction *DirCollection_;
		QAction *ViewProps_;

		bool ColumnsBeenResized_;
	public:
		FSBrowserWidget (QWidget* = 0);

		void AssociatePlayer (Player*);
	protected:
		void showEvent (QShowEvent*);
	private slots:
		void handleItemSelected (const QModelIndex&);
		void handleCollectionChanged ();
		void handleAddToCollection ();
		void handleRemoveFromCollection ();
		void loadFromFSBrowser ();
		void viewProps ();
	};
}
}
