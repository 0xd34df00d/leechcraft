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

#include "entrywrapper.h"
#include <ContactMessenger>
#include <PendingContactInfo>
#include <util/util.h>
#include <interfaces/azoth/azothutil.h>
#include "accountwrapper.h"
#include "astralityutil.h"
#include "msgwrapper.h"
#include "vcarddialog.h"

namespace LeechCraft
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
				SIGNAL (gotEntity (LeechCraft::Entity)),
				AW_,
				SIGNAL (gotEntity (LeechCraft::Entity)));

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

	QObject* EntryWrapper::GetObject ()
	{
		return this;
	}

	QObject* EntryWrapper::GetParentAccount () const
	{
		return AW_;
	}

	ICLEntry::Features EntryWrapper::GetEntryFeatures () const
	{
		return FPermanentEntry | FSupportsAuth | FSupportsGrouping;
	}

	ICLEntry::EntryType EntryWrapper::GetEntryType () const
	{
		return ETChat;
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
		Q_FOREACH (const QString& g, oldGroups)
			if (!groups.contains (g))
				C_->removeFromGroup (g);

		Q_FOREACH (const QString& g, groups)
			if (!oldGroups.contains (g))
				C_->addToGroup (g);
	}

	QStringList EntryWrapper::Variants () const
	{
		return QStringList (QString ());
	}

	QObject* EntryWrapper::CreateMessage (IMessage::MessageType mt, const QString&, const QString& body)
	{
		auto messenger = AW_->GetMessenger (GetHumanReadableID ());
		return new MsgWrapper (body, IMessage::DOut, messenger, this, mt);
	}

	QList<QObject*> EntryWrapper::GetAllMessages () const
	{
		QList<QObject*> result;
		Q_FOREACH (auto msg, AllMessages_)
			result << msg;
		return result;
	}

	void EntryWrapper::PurgeMessages (const QDateTime& before)
	{
		Util::StandardPurgeMessages (AllMessages_, before);
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

	QString EntryWrapper::GetRawInfo () const
	{
		return QString ();
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
			emit gotEntity (LeechCraft::Util::MakeNotification ("Azoth", msg, PCritical_));
			return;
		}

		auto info = qobject_cast<Tp::PendingContactInfo*> (op);
		auto fields = info->infoFields ();
		qDebug () << Q_FUNC_INFO << fields.allFields ().size ();
		Q_FOREACH (Tp::ContactInfoField field, fields.allFields ())
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
