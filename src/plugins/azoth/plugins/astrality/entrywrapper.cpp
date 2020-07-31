/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entrywrapper.h"
#include <ContactMessenger>
#include <PendingContactInfo>
#include <util/xpc/util.h>
#include <interfaces/azoth/azothutil.h>
#include "accountwrapper.h"
#include "astralityutil.h"
#include "msgwrapper.h"
#include "vcarddialog.h"

namespace LC
{
namespace Azoth
{
namespace Astrality
{
	EntryWrapper::EntryWrapper (Tp::ContactPtr c, AccountWrapper *aw)
	: QObject (aw)
	, AW_ (aw)
	, C_ (c)
	{
		connect (C_.data (),
				SIGNAL (presenceChanged (Tp::Presence)),
				this,
				SLOT (handlePresenceChanged ()));
		connect (C_.data (),
				SIGNAL (aliasChanged (QString)),
				this,
				SIGNAL (nameChanged (QString)));
		connect (C_.data (),
				SIGNAL (publishStateChanged (Tp::Contact::PresenceState, QString)),
				this,
				SLOT (handlePublishStateChanged (Tp::Contact::PresenceState, QString)));
		connect (C_.data (),
				SIGNAL (subscriptionStateChanged (Tp::Contact::PresenceState)),
				this,
				SLOT (handleSubStateChanged (Tp::Contact::PresenceState)));
		connect (C_.data (),
				SIGNAL (avatarDataChanged (Tp::AvatarData)),
				this,
				SLOT (handleAvatarDataChanged ()));

		C_->requestAvatarData ();

		connect (this,
				SIGNAL (gotEntity (LC::Entity)),
				AW_,
				SIGNAL (gotEntity (LC::Entity)));

		connect (AW_->GetMessenger (GetHumanReadableID ()).data (),
				SIGNAL (messageReceived (Tp::ReceivedMessage, Tp::TextChannelPtr)),
				this,
				SLOT (handleMessageReceived (Tp::ReceivedMessage, Tp::TextChannelPtr)));
	}

	void EntryWrapper::HandleMessage (MsgWrapper *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	Tp::ContactPtr EntryWrapper::GetContact () const
	{
		return C_;
	}

	QObject* EntryWrapper::GetQObject ()
	{
		return this;
	}

	IAccount* EntryWrapper::GetParentAccount () const
	{
		return AW_;
	}

	ICLEntry::Features EntryWrapper::GetEntryFeatures () const
	{
		return FPermanentEntry | FSupportsAuth | FSupportsGrouping;
	}

	ICLEntry::EntryType EntryWrapper::GetEntryType () const
	{
		return EntryType::Chat;
	}

	QString EntryWrapper::GetEntryName () const
	{
		return C_->id () == AW_->GetOurID () ?
				AW_->GetOurNick () :
				C_->alias ();
	}

	void EntryWrapper::SetEntryName (const QString&)
	{
	}

	QString EntryWrapper::GetEntryID () const
	{
		return AW_->GetAccountID () + "." + C_->id ();
	}

	QString EntryWrapper::GetHumanReadableID () const
	{
		return C_->id ();
	}

	QStringList EntryWrapper::Groups () const
	{
		return C_->groups ();
	}

	void EntryWrapper::SetGroups (const QStringList& groups)
	{
		const auto& oldGroups = Groups ();
		for (const auto& g : oldGroups)
			if (!groups.contains (g))
				C_->removeFromGroup (g);

		for (const auto& g : groups)
			if (!oldGroups.contains (g))
				C_->addToGroup (g);
	}

	QStringList EntryWrapper::Variants () const
	{
		return QStringList (QString ());
	}

	IMessage* EntryWrapper::CreateMessage (IMessage::Type mt, const QString&, const QString& body)
	{
		auto messenger = AW_->GetMessenger (GetHumanReadableID ());
		return new MsgWrapper (body, IMessage::Direction::Out, messenger, this, mt);
	}

	QList<IMessage*> EntryWrapper::GetAllMessages () const
	{
		QList<IMessage*> result;
		std::copy (AllMessages_.begin (), AllMessages_.end (), std::back_inserter (result));
		return result;
	}

	void EntryWrapper::PurgeMessages (const QDateTime& before)
	{
		AzothUtil::StandardPurgeMessages (AllMessages_, before);
	}

	void EntryWrapper::SetChatPartState (ChatPartState, const QString&)
	{
	}

	EntryStatus EntryWrapper::GetStatus (const QString&) const
	{
		return Status2Azoth (C_->presence ());
	}

	QImage EntryWrapper::GetAvatar () const
	{
		return QImage (C_->avatarData ().fileName);
	}

	void EntryWrapper::ShowInfo ()
	{
		connect (C_->requestInfo (),
				SIGNAL (finished (Tp::PendingOperation*)),
				this,
				SLOT (handleContactInfo (Tp::PendingOperation*)));
	}

	QList<QAction*> EntryWrapper::GetActions () const
	{
		return QList<QAction*> ();
	}

	QMap<QString, QVariant> EntryWrapper::GetClientInfo (const QString&) const
	{
		return QMap<QString, QVariant> ();
	}

	void EntryWrapper::MarkMsgsRead ()
	{
	}

	void EntryWrapper::ChatTabClosed ()
	{
	}

	AuthStatus EntryWrapper::GetAuthStatus () const
	{
		if (C_->publishState () == Tp::Contact::PresenceStateAsk)
			return ASContactRequested;

		int result = ASNone;
		if (C_->subscriptionState () == Tp::Contact::PresenceStateYes)
			result |= ASFrom;
		if (C_->publishState () == Tp::Contact::PresenceStateYes)
			result |= ASTo;
		return static_cast<AuthStatus> (result);
	}

	void EntryWrapper::ResendAuth (const QString& msg)
	{
		C_->authorizePresencePublication (msg);
	}

	void EntryWrapper::RevokeAuth (const QString& msg)
	{
		C_->removePresencePublication (msg);
	}

	void EntryWrapper::Unsubscribe (const QString& msg)
	{
		C_->removePresenceSubscription (msg);
	}

	void EntryWrapper::RerequestAuth (const QString& msg)
	{
		C_->requestPresenceSubscription (msg);
	}

	void EntryWrapper::handlePresenceChanged ()
	{
		emit statusChanged (GetStatus (QString ()), QString ());
	}

	void EntryWrapper::handleAvatarDataChanged ()
	{
		emit avatarChanged (GetAvatar ());
	}

	void EntryWrapper::handleContactInfo (Tp::PendingOperation *op)
	{
		VCardDialog *dia = new VCardDialog;
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->SetAvatar (GetAvatar ());
		dia->show ();

		if (op->isError ())
		{
			qWarning () << Q_FUNC_INFO
					<< op->errorName ()
					<< op->errorMessage ();
			const QString& msg = tr ("Error fetching contact info for %1: %2 (%3)")
					.arg (GetEntryName ())
					.arg (op->errorName ())
					.arg (op->errorMessage ());
			emit gotEntity (LC::Util::MakeNotification ("Azoth", msg, Priority::Critical));
			return;
		}

		auto info = qobject_cast<Tp::PendingContactInfo*> (op);
		auto fields = info->infoFields ();
		qDebug () << Q_FUNC_INFO << fields.allFields ().size ();
		for (const auto field : fields.allFields ())
			qDebug () << field.fieldName << field.fieldValue << field.parameters;

		dia->SetInfoFields (fields.allFields ());
	}

	void EntryWrapper::handlePublishStateChanged (Tp::Contact::PresenceState state, const QString& msg)
	{
		switch (state)
		{
		case Tp::Contact::PresenceStateNo:
			emit itemUnsubscribed (this, msg);
			break;
		case Tp::Contact::PresenceStateYes:
			emit itemSubscribed (this, msg);
			break;
		default:
			;
		}
	}

	void EntryWrapper::handleSubStateChanged (Tp::Contact::PresenceState state)
	{
		switch (state)
		{
		case Tp::Contact::PresenceStateNo:
			emit itemCancelledSubscription (this, QString ());
			break;
		case Tp::Contact::PresenceStateYes:
			emit itemGrantedSubscription (this, QString ());
			break;
		default:
			;
		}
	}

	void EntryWrapper::handleMessageReceived (const Tp::ReceivedMessage& tpMsg, Tp::TextChannelPtr)
	{
		qDebug () << Q_FUNC_INFO << GetHumanReadableID ()
				<< tpMsg.isScrollback () << tpMsg.isDeliveryReport ();
		if (tpMsg.isScrollback ())
			return;

		if (tpMsg.isDeliveryReport ())
			return;

		auto msg = new MsgWrapper (tpMsg, AW_->GetMessenger (GetHumanReadableID ()), this);
		HandleMessage (msg);
	}
}
}
}
