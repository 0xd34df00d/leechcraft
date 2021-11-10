/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entrybase.h"
#include <QAction>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/azothutil.h>
#include "clientconnection.h"
#include "ircprotocol.h"
#include "ircaccount.h"
#include "ircmessage.h"
#include "vcarddialog.h"
#include "ircparticipantentry.h"

namespace LC::Azoth::Acetamide
{
	EntryBase::EntryBase (IrcAccount *account)
	: QObject { account }
	, Account_ { account }
	{
	}

	EntryBase::~EntryBase ()
	{
		qDeleteAll (AllMessages_);
		qDeleteAll (Actions_);
		delete VCardDialog_;
	}

	QObject* EntryBase::GetQObject ()
	{
		return this;
	}

	QList<IMessage*> EntryBase::GetAllMessages () const
	{
		return AllMessages_;
	}

	void EntryBase::PurgeMessages (const QDateTime& before)
	{
		AzothUtil::StandardPurgeMessages (AllMessages_, before);
	}

	void EntryBase::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus EntryBase::GetStatus (const QString&) const
	{
		return CurrentStatus_;
	}

	QList<QAction*> EntryBase::GetActions () const
	{
		return Actions_;
	}

	void EntryBase::ShowInfo ()
	{
		const auto entry = qobject_cast<IrcParticipantEntry*> (this);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< this
					<< "is not an IrcParticipantEntry object";
			return;
		}

		if (!VCardDialog_)
			VCardDialog_ = new VCardDialog ();

		Account_->GetClientConnection ()->FetchVCard (entry->GetServerID (),
				entry->GetEntryName());
		VCardDialog_->show ();
	}

	QMap<QString, QVariant> EntryBase::GetClientInfo (const QString&) const
	{
		return {};
	}

	void EntryBase::MarkMsgsRead ()
	{
		Account_->GetParentProtocol ()->GetProxyObject ()->MarkMessagesAsRead (this);
	}

	void EntryBase::ChatTabClosed ()
	{
	}

	void EntryBase::HandleMessage (IrcMessage *msg)
	{
		msg->SetOtherPart (this);
		Account_->GetParentProtocol ()->GetProxyObject ()->GetFormatterProxy ().PreprocessMessage (msg);

		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void EntryBase::SetStatus (const EntryStatus& status)
	{
		CurrentStatus_ = status;
		emit statusChanged (CurrentStatus_, QString ());
	}

	void EntryBase::SetAvatar (const QByteArray&)
	{
	}

	void EntryBase::SetAvatar (const QImage&)
	{
	}

	void EntryBase::SetInfo (const WhoIsMessage& msg)
	{
		if (VCardDialog_)
			VCardDialog_->UpdateInfo (msg);
	}
}
