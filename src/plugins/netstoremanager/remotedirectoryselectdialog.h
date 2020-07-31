/**********************************************************************
 *  LeechCraft - modular cross-platform feature rich internet client.
 *  Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_remotedirectoryselectdialog.h"
#include "interfaces/netstoremanager/isupportfilelistings.h"

class QStandardItemModel;

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;
	class FilesProxyModel;

	class RemoteDirectorySelectDialog : public QDialog
	{
		Q_OBJECT

		Ui::RemoteDirectorySelectDialog Ui_;
		QByteArray AccountId_;
		QStandardItemModel *Model_;
		FilesProxyModel *ProxyModel_;
		AccountsManager *AM_;

	public:
		explicit RemoteDirectorySelectDialog (const QByteArray& accountId,
				AccountsManager *am, QWidget *parent = 0);

		QStringList GetDirectoryPath () const;
	private:
		void HandleGotListing (const ISupportFileListings::RefreshResult_t& items);
	private slots:
		void createNewDir ();
	};
}
}
