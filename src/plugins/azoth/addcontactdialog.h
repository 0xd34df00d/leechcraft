/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_ADDCONTACTDIALOG_H
#define PLUGINS_AZOTH_ADDCONTACTDIALOG_H
#include <QDialog>
#include "ui_addcontactdialog.h"

namespace LC
{
namespace Azoth
{
	class IAccount;

	class AddContactDialog : public QDialog
	{
		Q_OBJECT

		Ui::AddContactDialog Ui_;
	public:
		AddContactDialog (IAccount *focusAcc = 0, QWidget* = 0);

		void SetContactID (const QString&);
		void SetNick (const QString&);

		IAccount* GetSelectedAccount () const;
		QString GetContactID () const;
		QString GetNick () const;
		QString GetReason () const;
		QStringList GetGroups () const;
	private slots:
		void on_Protocol__currentIndexChanged (int);
		void checkComplete ();
	private:
		void FocusAccount (IAccount*);
	};
}
}

#endif
