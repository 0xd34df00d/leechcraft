/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/blogique/ibloggingplatform.h>
#include "ui_accountconfigurationwidget.h"

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	class AccountConfigurationWidget : public QWidget
	{
		Q_OBJECT

		Ui::AccountConfigurationWidget Ui_;
		IBloggingPlatform::AccountAddOptions Option_;

		const QString SuggestedPath_;
	public:
		AccountConfigurationWidget (QWidget* = 0,
				IBloggingPlatform::AccountAddOptions = IBloggingPlatform::AAONoOptions,
				const QString& = QString ());

		void SetAccountBasePath (const QString& path);
		QString GetAccountBasePath () const;
		IBloggingPlatform::AccountAddOptions GetOption () const;

	private slots:
		void on_OpenAccountBase__released ();
	};
}
}
}
