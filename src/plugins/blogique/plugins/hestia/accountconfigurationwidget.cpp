/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "accountconfigurationwidget.h"
#include <QFileDialog>

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	AccountConfigurationWidget::AccountConfigurationWidget (QWidget *parent,
			IBloggingPlatform::AccountAddOptions option)
	: QWidget (parent)
	, Option_ (option)
	{
		Ui_.setupUi (this);
		Ui_.Label_->setText (Option_ & IBloggingPlatform::AAORegisterNewAccount ?
			tr ("Select new account base") :
			tr ("Open existing account base"));
	}

	void AccountConfigurationWidget::on_OpenAccountBase__released ()
	{
		QString path = (Option_ & IBloggingPlatform::AAORegisterNewAccount) ?
			QFileDialog::getSaveFileName (this,
					tr ("Select account base"),
					QDir::homePath (),
					tr ("Account bases (*.db)")) :
			QFileDialog::getOpenFileName (this,
					tr ("Open account base"),
					QDir::homePath (),
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
