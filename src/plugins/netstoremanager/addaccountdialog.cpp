/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "addaccountdialog.h"
#include "interfaces/netstoremanager/istorageplugin.h"
#include "utils.h"

namespace LC
{
namespace NetStoreManager
{
	AddAccountDialog::AddAccountDialog (const QList<IStoragePlugin*>& plugins, QWidget *w)
	: QDialog (w)
	{
		Ui_.setupUi (this);

		for (auto plugin : plugins)
		{
			const auto& name = plugin->GetStorageName ();
			const auto& icon = Utils::GetStorageIcon (plugin);
			Ui_.Storage_->addItem (icon, name, QVariant::fromValue (plugin));
		}
	}

	QString AddAccountDialog::GetAccountName () const
	{
		return Ui_.AccountName_->text ();
	}

	IStoragePlugin* AddAccountDialog::GetStoragePlugin () const
	{
		const int idx = Ui_.Storage_->currentIndex ();
		if (idx < 0)
			return 0;

		return Ui_.Storage_->itemData (idx).value<IStoragePlugin*> ();
	}
}
}
