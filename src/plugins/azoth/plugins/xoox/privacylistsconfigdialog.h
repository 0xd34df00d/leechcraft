/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_privacylistsconfigdialog.h"
#include "xeps/privacylistsmanager.h"

class QStandardItemModel;
class QStandardItem;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class PrivacyListsManager;

	class PrivacyListsConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::PrivacyListsConfigDialog Ui_;
		PrivacyListsManager * const Manager_;
		QMap<QString, PrivacyList> Lists_;

		QStandardItemModel * const Model_;
	public:
		PrivacyListsConfigDialog (PrivacyListsManager*, QWidget* = 0);
	private:
		void QueryLists ();
		void QueryList (const QString&);
		void AddListToBoxes (const QString&);
		void ReinitModel ();
		QList<QStandardItem*> ToRow (const PrivacyListItem&) const;

		void HandleGotList (const PrivacyList&);
	public slots:
		void accept () override;
		void reject () override;
	private slots:
		void on_ConfigureList__activated (int);
		void on_AddButton__released ();
		void on_RemoveButton__released ();
		void on_DefaultPolicy__currentIndexChanged (int);
		void on_AddRule__released ();
		void on_ModifyRule__released ();
		void on_RemoveRule__released ();
		void on_MoveUp__released ();
		void on_MoveDown__released ();

		void handleGotLists (const QStringList&, const QString&, const QString&);
		void handleError (const QString&);
	};
}
}
}
