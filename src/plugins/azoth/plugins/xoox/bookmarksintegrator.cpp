/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarksintegrator.h"
#include <QTimer>
#include <QXmppBookmarkManager.h>
#include <QXmppBookmarkSet.h>
#include <interfaces/azoth/iproxyobject.h>
#include "clientconnection.h"
#include "glooxaccount.h"
#include "roomclentry.h"

namespace LC::Azoth::Xoox
{
	BookmarksIntegrator::BookmarksIntegrator (QXmppBookmarkManager& mgr,
			ClientConnection& conn, GlooxAccount& acc, QObject *parent)
	: QObject { parent }
	, Acc_ { acc }
	, Conn_ { conn }
	, Mgr_ { mgr }
	{
		connect (&conn,
				&ClientConnection::connected,
				[this]
				{
					connect (&Mgr_,
							&QXmppBookmarkManager::bookmarksReceived,
							this,
							&BookmarksIntegrator::HandleBookmarksReceived,
							Qt::UniqueConnection);
				});

		connect (&Mgr_,
				&QXmppBookmarkManager::bookmarksReceived,
				&Acc_,
				&GlooxAccount::bookmarksChanged);
	}

	QVariantList BookmarksIntegrator::GetBookmarkedMUCs () const
	{
		QVariantList result;

		for (const auto& conf : Mgr_.bookmarks ().conferences ())
		{
			const auto& split = conf.jid ().split ('@', Qt::SkipEmptyParts);
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
			cm ["AccountID"] = Acc_.GetAccountID ();
			cm ["Nick"] = conf.nickName ();
			cm ["Room"] = split.at (0);
			cm ["Server"] = split.at (1);
			cm ["Autojoin"] = conf.autoJoin ();
			cm ["StoredName"] = conf.name ();
			result << cm;
		}

		return result;
	}

	void BookmarksIntegrator::SetBookmarkedMUCs (const QVariantList& datas)
	{
		QSet<QString> jids;

		QList<QXmppBookmarkConference> mucs;
		for (const auto& var : datas)
		{
			const QVariantMap& map = var.toMap ();
			QXmppBookmarkConference conf;
			conf.setAutoJoin (map.value ("Autojoin").toBool ());

			const auto& room = map.value ("Room").toString ();
			const auto& server = map.value ("Server").toString ();
			if (room.isEmpty () || server.isEmpty ())
				continue;

			const auto& jid = room + '@' + server;
			if (jids.contains (jid))
				continue;

			jids << jid;

			conf.setJid (jid);
			conf.setNickName (map.value ("Nick").toString ());
			conf.setName (map.value ("StoredName").toString ());
			mucs << conf;
		}

		auto set = Mgr_.bookmarks ();
		set.setConferences (mucs);
		Mgr_.setBookmarks (set);
	}

	void BookmarksIntegrator::HandleBookmarksReceived (const QXmppBookmarkSet& set)
	{
		disconnect (&Mgr_,
				&QXmppBookmarkManager::bookmarksReceived,
				this,
				&BookmarksIntegrator::HandleBookmarksReceived);

		for (const auto& conf : set.conferences ())
		{
			if (!conf.autoJoin ())
				continue;

			const JoinQueueItem item
			{
				true,
				conf.jid (),
				conf.nickName ()
			};
			JoinQueue_ << item;
		}

		if (!JoinQueue_.isEmpty ())
			QTimer::singleShot (3000,
					this,
					&BookmarksIntegrator::HandleAutojoinQueue);
	}

	void BookmarksIntegrator::HandleAutojoinQueue ()
	{
		if (JoinQueue_.isEmpty ())
			return;

		if (!Acc_.GetParentProtocol ()->GetProxyObject ()->IsAutojoinAllowed ())
			return;

		const auto& it = JoinQueue_.takeFirst ();
		if (const auto roomItem = Conn_.JoinRoom (it.RoomJID_, it.Nickname_, it.AsAutojoin_))
			emit Acc_.gotCLItems ({ roomItem });

		if (!JoinQueue_.isEmpty ())
			QTimer::singleShot (800,
					this,
					&BookmarksIntegrator::HandleAutojoinQueue);
	}
}
