/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_accountconfigdialog.h"
#include "accountconfig.h"

namespace LC
{
namespace Snails
{
	class AccountConfigDialog : public QDialog
	{
		Q_OBJECT

		Ui::AccountConfigDialog Ui_;
	public:
		AccountConfigDialog (QWidget* = 0);

		AccountConfig GetConfig () const;
		void SetConfig (const AccountConfig&);

		void SetAllFolders (const QList<QStringList>&);
		QList<QStringList> GetFoldersToSync () const;
		void SetFoldersToSync (const QList<QStringList>&);
		QStringList GetOutFolder () const;
		void SetOutFolder (const QStringList&);
	private slots:
		void resetInPort ();
		void rebuildFoldersToSyncLine ();
	};
}
}
