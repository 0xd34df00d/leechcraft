/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "joingroupchatwidget.h"
#include <QtDebug>
#include "glooxaccount.h"
#include "glooxprotocol.h"
#include "core.h"

namespace LC
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

	QString JoinGroupchatWidget::GetPassword () const
	{
		return Ui_.Password_->text ();
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
		acc->JoinRoom (GetServer (), GetRoom (), GetNickname (), GetPassword ());
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
		result ["Password"] = GetPassword ();
		return result;
	}

	void JoinGroupchatWidget::SetIdentifyingData (const QVariantMap& data)
	{
		const auto& nick = data ["Nick"].toString ();
		const auto& room = data ["Room"].toString ();
		const auto& server = data ["Server"].toString ();
		const auto& password = data ["Password"].toString ();

		if (!nick.isEmpty ())
			Ui_.Nickname_->setText (nick);
		if (!room.isEmpty ())
			Ui_.Room_->setText (room);
		if (!server.isEmpty ())
			Ui_.Server_->setText (server);
		if (!password.isEmpty ())
			Ui_.Password_->setText (password);

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
