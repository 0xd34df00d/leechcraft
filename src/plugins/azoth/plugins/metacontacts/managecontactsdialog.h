/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_MANAGECONTACTSDIALOG_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_MANAGECONTACTSDIALOG_H
#include <QDialog>
#include "ui_managecontactsdialog.h"

class QStandardItemModel;

namespace LC
{
namespace Azoth
{
namespace Metacontacts
{
	class ManageContactsDialog : public QDialog
	{
		Q_OBJECT
		
		Ui::ManageContactsDialog Ui_;
		QStandardItemModel *Model_;
	public:
		ManageContactsDialog (const QList<QObject*>&, QWidget* = 0);
		
		QList<QObject*> GetObjects () const;
	private slots:
		void on_MoveUp__released ();
		void on_MoveDown__released ();
		void on_Remove__released ();
	};
}
}
}

#endif
