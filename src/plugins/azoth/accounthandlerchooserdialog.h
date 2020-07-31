/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include "ui_accounthandlerchooserdialog.h"

namespace LC
{
namespace Azoth
{
	class IAccount;

	class AccountHandlerChooserDialog : public QDialog
	{
		Q_OBJECT

		Ui::AccountHandlerChooserDialog Ui_;
	public:
		AccountHandlerChooserDialog (const QList<IAccount*>&, const QString&, QWidget* = 0);

		IAccount* GetSelectedAccount () const;
	};
}
}
