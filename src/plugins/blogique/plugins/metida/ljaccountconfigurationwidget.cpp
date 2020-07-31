/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "ljaccountconfigurationwidget.h"

namespace LC
{
namespace Blogique
{
namespace Metida
{
	LJAccountConfigurationWidget::LJAccountConfigurationWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	QString LJAccountConfigurationWidget::GetLogin () const
	{
		return Ui_.Login_->text ();
	}

	void LJAccountConfigurationWidget::SetLogin (const QString& login)
	{
		Ui_.Login_->setText (login);
	}

	QString LJAccountConfigurationWidget::GetPassword () const
	{
		return Ui_.Password_->text ();
	}

	void LJAccountConfigurationWidget::SetPassword (const QString& pass)
	{
		Ui_.Password_->setText (pass);
	}

}
}
}
