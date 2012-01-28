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

#include "joingroupchatwidget.h"
#include <QtDebug>
#include "glooxaccount.h"
#include "glooxprotocol.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	JoinGroupchatWidget::JoinGroupchatWidget (QWidget *parent)
	: QWidget (parent)
	, SelectedAccount_ (0)
	{
		Ui_.setupUi (this);
		connect (Ui_.Nickname_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (checkValidity ()));
		connect (Ui_.Server_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (checkValidity ()));
		connect (Ui_.Room_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (checkValidity ()));
	}

	QString JoinGroupchatWidget::GetNickname () const
	{
		return Ui_.Nickname_->text ();
	}

	QString JoinGroupchatWidget::GetRoom () const
	{
		return Ui_.Room_->text ();
	}

	QString JoinGroupchatWidget::GetServer () const
	{
		return Ui_.Server_->text ();
	}

	void JoinGroupchatWidget::AccountSelected (QObject *accObj)
	{
		GlooxAccount *acc = qobject_cast<GlooxAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< accObj
					<< "to GlooxAccount";
			return;
		}

		SelectedAccount_ = acc;
		Ui_.Nickname_->setText (acc->GetOurNick ());
	}

	void JoinGroupchatWidget::Join (QObject *accObj)
	{
		GlooxAccount *acc = qobject_cast<GlooxAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< accObj
					<< "to GlooxAccount";
			return;
		}

		SelectedAccount_ = acc;
		acc->JoinRoom (GetServer (), GetRoom (), GetNickname ());
	}

	void JoinGroupchatWidget::Cancel ()
	{
	}

	QVariantMap JoinGroupchatWidget::GetIdentifyingData () const
	{
		QVariantMap result;
		result ["HumanReadableName"] = QString ("%2@%3 (%1)")
				.arg (GetNickname ())
				.arg (GetRoom ())
				.arg (GetServer ());
		result ["AccountID"] = SelectedAccount_->GetAccountID ();
		result ["Nick"] = GetNickname ();
		result ["Room"] = GetRoom ();
		result ["Server"] = GetServer ();
		return result;
	}

	void JoinGroupchatWidget::SetIdentifyingData (const QVariantMap& data)
	{
		const QString& nick = data ["Nick"].toString ();
		const QString& room = data ["Room"].toString ();
		const QString& server = data ["Server"].toString ();

		if (!nick.isEmpty ())
			Ui_.Nickname_->setText (nick);
		if (!room.isEmpty ())
			Ui_.Room_->setText (room);
		if (!server.isEmpty ())
			Ui_.Server_->setText (server);

		checkValidity ();
	}

	void JoinGroupchatWidget::checkValidity ()
	{
		bool notOk = Ui_.Nickname_->text ().isEmpty () ||
				Ui_.Room_->text ().isEmpty () ||
				Ui_.Server_->text ().isEmpty ();
		emit validityChanged (!notOk);
	}

	void JoinGroupchatWidget::on_ViewRooms__released ()
	{
		if (!SelectedAccount_)
			return;

		const QString& server = Ui_.Server_->text ();
		SelectedAccount_->CreateSDForResource (server);
	}

	void JoinGroupchatWidget::on_Server__textChanged (const QString& str)
	{
		Ui_.ViewRooms_->setEnabled (SelectedAccount_ && !str.isEmpty ());
	}
}
}
}