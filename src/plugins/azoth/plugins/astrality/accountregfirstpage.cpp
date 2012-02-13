/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "accountregfirstpage.h"
#include <TelepathyQt/ProtocolInfo>

namespace LeechCraft
{
namespace Azoth
{
namespace Astrality
{
	AccountRegFirstPage::AccountRegFirstPage (const Tp::ProtocolInfo& info, bool regNew, QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		auto visRow = [&info] (QString parm, QWidget *w1, QWidget *w2 = 0)
		{
			if (!info.hasParameter (parm))
			{
				w1->hide ();
				if (w2)
					w2->hide ();
			}
		};
		visRow ("account", Ui_.AccIDLabel_, Ui_.AccID_);
		visRow ("server", Ui_.ServerLabel_, Ui_.Server_);
		visRow ("port", Ui_.PortLabel_, Ui_.Port_);
		visRow ("require-encryption", Ui_.RequireEncryption_);

		if (!regNew)
		{
			Ui_.PasswordLabel_->hide ();
			Ui_.Password_->hide ();
		}
	}

	QString AccountRegFirstPage::GetAccountID () const
	{
		return Ui_.AccID_->text ();
	}

	QString AccountRegFirstPage::GetPassword () const
	{
		return Ui_.Password_->text ();
	}

	QString AccountRegFirstPage::GetServer () const
	{
		return Ui_.Server_->text ();
	}

	int AccountRegFirstPage::GetPort () const
	{
		return Ui_.Port_->value ();
	}

	bool AccountRegFirstPage::ShouldRequireEncryption () const
	{
		return Ui_.RequireEncryption_->isChecked ();
	}
}
}
}
