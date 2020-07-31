/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_ADDACCOUNTDIALOG_H
#define PLUGINS_NETSTOREMANAGER_ADDACCOUNTDIALOG_H
#include <QDialog>
#include "ui_addaccountdialog.h"

namespace LC
{
namespace NetStoreManager
{
	class IStoragePlugin;

	class AddAccountDialog : public QDialog
	{
		Q_OBJECT

		Ui::AddAccountDialog Ui_;
	public:
		AddAccountDialog (const QList<IStoragePlugin*>&, QWidget* = 0);

		QString GetAccountName () const;
		IStoragePlugin* GetStoragePlugin () const;
	};
}
}

#endif
