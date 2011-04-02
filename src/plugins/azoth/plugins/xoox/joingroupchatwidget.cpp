/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

	QVariantList JoinGroupchatWidget::GetBookmarkedMUCs () const
	{
		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Core::Instance ().GetProtocols ().at (0));
		QVariantList result;
		Q_FOREACH (QObject *obj, proto->GetRegisteredAccounts ())
		{
			GlooxAccount *acc = qobject_cast<GlooxAccount*> (obj);
			const QXmppBookmarkSet& set = acc->GetBookmarks ();
			Q_FOREACH (const QXmppBookmarkConference& conf, set.conferences ())
			{
				const QStringList& split = conf.jid ().split ('@', QString::SkipEmptyParts);
				if (split.size () != 2)
				{
					qWarning () << Q_FUNC_INFO
							<< "incorrectly split jid for conf"
							<< conf.jid ()
							<< split;
					continue;
				}

				QVariantMap cm;
				cm ["HumanReadableName"] = QString ("%1 (%2)")
						.arg (conf.jid ())
						.arg (conf.nickName ());
				cm ["AccountID"] = acc->GetAccountID ();
				cm ["Nick"] = conf.nickName ();
				cm ["Room"] = split.at (0);
				cm ["Server"] = split.at (1);
				cm ["Autojoin"] = conf.autoJoin ();
				cm ["StoredName"] = conf.name ();
				result << cm;
			}
		}
		return result;
	}
	
	void JoinGroupchatWidget::SetBookmarkedMUCs (QObject *accObj, const QVariantList& datas)
	{
		GlooxAccount *acc = qobject_cast<GlooxAccount*> (accObj);
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "is not a GlooxAccount";
			return;
		}

		QList<QXmppBookmarkConference> mucs;
		Q_FOREACH (const QVariant& var, datas)
		{
			const QVariantMap& map = var.toMap ();
			QXmppBookmarkConference conf;
			qDebug () << map;
			conf.setAutoJoin (map.value ("Autojoin").toBool ());
			conf.setJid (map.value ("Room").toString () + '@' + map.value ("Server").toString ());
			conf.setNickName (map.value ("Nick").toString ());
			conf.setName (map.value ("StoredName").toString ());
			mucs << conf;
		}

		QXmppBookmarkSet set;
		set.setConferences (mucs);
		set.setUrls (acc->GetBookmarks ().urls ());
		acc->SetBookmarks (set);
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