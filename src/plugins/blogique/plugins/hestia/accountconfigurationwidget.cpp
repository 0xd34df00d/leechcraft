/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "accountconfigurationwidget.h"
#include <QFileDialog>
#include <util/util.h>
#include <util/sys/paths.h>

namespace LC
{
namespace Blogique
{
namespace Hestia
{
	AccountConfigurationWidget::AccountConfigurationWidget (QWidget *parent,
			IBloggingPlatform::AccountAddOptions option,
			const QString& accName)
	: QWidget (parent)
	, Option_ (option)
	, SuggestedPath_ (accName.isEmpty () ?
				QString () :
				Util::CreateIfNotExists ("blogique/hestia").absoluteFilePath (accName + ".db"))
	{
		Ui_.setupUi (this);
		Ui_.Label_->setText (Option_ & IBloggingPlatform::AAORegisterNewAccount ?
			tr ("Select new account base") :
			tr ("Open existing account base"));
		Ui_.AccountBasePath_->setText (SuggestedPath_);
	}

	void AccountConfigurationWidget::on_OpenAccountBase__released ()
	{
		QString path = (Option_ & IBloggingPlatform::AAORegisterNewAccount) ?
			QFileDialog::getSaveFileName (this,
					tr ("Select account base"),
					SuggestedPath_,
					tr ("Account bases (*.db)")) :
			QFileDialog::getOpenFileName (this,
					tr ("Open account base"),
					SuggestedPath_,
					tr ("Account bases (*.db)"));

		if (path.isEmpty ())
			return;

		SetAccountBasePath (path);
	}

	void AccountConfigurationWidget::SetAccountBasePath (const QString& path)
	{
		Ui_.AccountBasePath_->setText (path);
	}

	QString AccountConfigurationWidget::GetAccountBasePath () const
	{
		return Ui_.AccountBasePath_->text ();
	}

	IBloggingPlatform::AccountAddOptions AccountConfigurationWidget::GetOption () const
	{
		return Option_;
	}

}
}
}
