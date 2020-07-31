/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "commentsmanager.h"
#include <QTimer>
#include "interfaces/blogique/ibloggingplatform.h"
#include "core.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Blogique
{
	CommentsManager::CommentsManager (QObject *parent)
	: QObject (parent)
	, CommentsCheckingTimer_ (new QTimer (this))
	{
		XmlSettingsManager::Instance ().RegisterObject ("CheckingCommentsEnabled",
				this, "handleCommentsCheckingChanged");
		XmlSettingsManager::Instance ().RegisterObject ("UpdateCommentsInterval",
				this, "handleCommentsCheckingTimerChanged");
		connect (CommentsCheckingTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkForComments ()));
		handleCommentsCheckingTimerChanged ();
	}

	QList<CommentEntry> CommentsManager::GetComments () const
	{
		return RecentComments_.values ();
	}

	void CommentsManager::checkForComments ()
	{
		for (auto acc : Core::Instance ().GetAccounts ())
			acc->RequestRecentComments ();
	}

	void CommentsManager::handleCommentsCheckingChanged ()
	{
		if (!XmlSettingsManager::Instance ().Property ("CheckingCommentsEnabled", true).toBool () &&
				CommentsCheckingTimer_->isActive ())
			CommentsCheckingTimer_->stop ();
	}

	void CommentsManager::handleCommentsCheckingTimerChanged ()
	{
		if (XmlSettingsManager::Instance ().Property ("CheckingCommentsEnabled", true).toBool ())
			CommentsCheckingTimer_->start (XmlSettingsManager::Instance ()
					.property ("UpdateCommentsInterval").toInt () * 60 * 1000);
		else if (CommentsCheckingTimer_->isActive ())
			CommentsCheckingTimer_->stop ();
	}

	void CommentsManager::handleGotRecentComments (const QList<CommentEntry>& comments)
	{
		for (const auto& comment : comments)
			RecentComments_ << comment;
		emit commentsUpdated ();
	}

	void CommentsManager::handleCommentsDeleted (const QList<qint64>& ids)
	{
		auto account = qobject_cast<IAccount*> (sender ());
		if (!account)
			return;

		for (auto id : ids)
		{
			CommentEntry ce;
			ce.AccountID_ = account->GetAccountID ();
			ce.CommentID_ = id;
			RecentComments_.remove (ce);
		}
		emit commentsUpdated ();
	}

}
}
