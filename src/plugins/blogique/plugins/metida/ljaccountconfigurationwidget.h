/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_ljaccountconfigurationwidget.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	class LJAccountConfigurationWidget : public QWidget
	{
		Q_OBJECT

		Ui::LJAccountConfigurationWidget Ui_;
	public:
		LJAccountConfigurationWidget (QWidget *parent = 0);

		QString GetLogin () const;
		void SetLogin (const QString& login);

		QString GetPassword () const;
		void SetPassword (const QString& pass);
	};
}
}
}
