/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toxcontact.h"
#include <QImage>
#include <interfaces/azoth/azothutil.h>
#include "toxaccount.h"
#include "chatmessage.h"

namespace LC::Azoth::Sarin
{
	ToxContact::ToxContact (const QByteArray& pubkey, ToxAccount *account)
	: QObject { account }
	, Pubkey_ { pubkey }
	, Acc_ { account }
	{
	}

	const QByteArray& ToxContact::GetPubKey () const
	{
		return Pubkey_;
	}

	QObject* ToxContact::GetQObject ()
	{
		return this;
	}

	IAccount* ToxContact::GetParentAccount () const
	{
		return Acc_;
	}

	ICLEntry::Features ToxContact::GetEntryFeatures () const
	{
		return FPermanentEntry;
	}

	ICLEntry::EntryType ToxContact::GetEntryType () const
	{
		return EntryType::Chat;
	}

	QString ToxContact::GetEntryName () const
	{
		return PublicName_.isEmpty () ? Pubkey_ : PublicName_;
	}

	void ToxContact::SetEntryName (const QString& name)
	{
		if (name == PublicName_)
			return;

		PublicName_ = name;
		emit nameChanged (GetEntryName ());
	}

	QString ToxContact::GetEntryID () const
	{
		return Acc_->GetAccountID () + '/' + Pubkey_;
	}

	QString ToxContact::GetHumanReadableID () const
	{
		return Pubkey_;
	}

	QStringList ToxContact::Groups () const
	{
		return {};
	}

	void ToxContact::SetGroups (const QStringList&)
	{
	}

	QStringList ToxContact::Variants () const
	{
		return { "" };
	}

	IMessage* ToxContact::CreateMessage (IMessage::Type type, const QString&, const QString& body)
	{
		if (type != IMessage::Type::ChatMessage)
		{
			qWarning () << Q_FUNC_INFO
					<< "unsupported message type"
					<< static_cast<int> (type);
			return nullptr;
		}

		return new ChatMessage { body, IMessage::Direction::Out, this };
	}

	QList<IMessage*> ToxContact::GetAllMessages () const
	{
		QList<IMessage*> result;
		std::copy (AllMessages_.begin (), AllMessages_.end (), std::back_inserter (result));
		return result;
	}

	void ToxContact::PurgeMessages (const QDateTime& before)
	{
		AzothUtil::StandardPurgeMessages (AllMessages_, before);
	}

	void ToxContact::SetChatPartState (ChatPartState state, const QString&)
	{
		Acc_->SetTypingState (Pubkey_, state == CPSComposing);
	}

	EntryStatus ToxContact::GetStatus (const QString&) const
	{
		return Status_;
	}

	void ToxContact::ShowInfo ()
	{
	}

	QList<QAction*> ToxContact::GetActions () const
	{
		return {};
	}

	QMap<QString, QVariant> ToxContact::GetClientInfo (const QString&) const
	{
		return {};
	}

	void ToxContact::MarkMsgsRead ()
	{
	}

	void ToxContact::ChatTabClosed ()
	{
	}

	void ToxContact::SetStatus (const EntryStatus& status)
	{
		if (status == Status_)
			return;

		Status_ = status;
		emit statusChanged (Status_, Variants ().front ());
	}

	void ToxContact::SetTyping (bool isTyping)
	{
		emit chatPartStateChanged (isTyping ? ChatPartState::CPSComposing : ChatPartState::CPSNone, {});
	}

	void ToxContact::HandleMessage (ChatMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void ToxContact::SendMessage (ChatMessage *msg)
	{
		Acc_->SendMessage (Pubkey_, msg);
	}
}
