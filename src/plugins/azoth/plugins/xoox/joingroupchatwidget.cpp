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

#include "joingroupchatwidget.h"
#include <QtDebug>
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
	JoinGroupchatWidget::JoinGroupchatWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
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

		acc->JoinRoom (GetServer (), GetRoom (), GetNickname ());
	}

	void JoinGroupchatWidget::Cancel ()
	{
	}

	QVariantMap JoinGroupchatWidget::GetIdentifyingData () const
	{
		QVariantMap result;
		result ["HumanReadableName"] = QString ("%1 on %2@%3")
				.arg (GetNickname ())
				.arg (GetRoom ())
				.arg (GetServer ());
		result ["Nick"] = GetNickname ();
		result ["Room"] = GetRoom ();
		result ["Server"] = GetServer ();
		return result;
	}

	QVariantList JoinGroupchatWidget::GetBookmarkedMUCs () const
	{
		return QVariantList ();
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
	}
}
}
}
}
}