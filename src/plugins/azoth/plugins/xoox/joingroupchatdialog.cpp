/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "joingroupchatdialog.h"
#include <gloox/jid.h>
#include "glooxaccount.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	JoinGroupchatDialog::JoinGroupchatDialog (const QList<GlooxAccount*>& accounts,
			QWidget *parent)
	: QDialog (parent)
	, Accounts_ (accounts)
	{
		Ui_.setupUi (this);

		if (!accounts.size ())
			return;

		Q_FOREACH (GlooxAccount *acc, accounts)
		{
			QString text = QString ("%1 (%2)")
					.arg (acc->GetAccountName ())
					.arg (acc->GetJID ());
			Ui_.Account_->addItem (text);
		}

		on_Account__currentIndexChanged (0);
	}

	QString JoinGroupchatDialog::GetNickname () const
	{
		return Ui_.Nickname_->text ();
	}

	QString JoinGroupchatDialog::GetRoom () const
	{
		return Ui_.Room_->text ();
	}

	QString JoinGroupchatDialog::GetServer () const
	{
		return Ui_.Server_->text ();
	}

	GlooxAccount* JoinGroupchatDialog::GetSelectedAccount () const
	{
		int idx = Ui_.Account_->currentIndex ();
		if (idx >= 0)
			return Accounts_.at (idx);
		else
			return 0;
	}

	void JoinGroupchatDialog::on_Account__currentIndexChanged (int index)
	{
		GlooxAccount *acc = Accounts_.at (index);
		Ui_.Nickname_->setText (acc->GetNick ());
	}
}
}
}
}
}