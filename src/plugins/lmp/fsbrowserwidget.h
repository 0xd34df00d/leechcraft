/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QWidget>
#include "ui_fsbrowserwidget.h"

namespace LC::LMP
{
	class Player;
	class FSModel;

	class FSBrowserWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::FSBrowserWidget)

		Ui::FSBrowserWidget Ui_;

		Player *Player_ = nullptr;
		FSModel * const FSModel_;
		QAction * const DirCollection_;
		QAction * const ViewProps_;

		QMetaObject::Connection DirCollectionConn_;

		bool ColumnsBeenResized_ = false;
	public:
		explicit FSBrowserWidget (QWidget* = nullptr);

		void AssociatePlayer (Player*);
	protected:
		void showEvent (QShowEvent*) override;
	private:
		void UpdateActions (const QModelIndex&);
		void LoadFromFSBrowser ();
	};
}
