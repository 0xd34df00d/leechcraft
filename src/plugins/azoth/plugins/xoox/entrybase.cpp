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

#include "entrybase.h"
#include <QImage>
#include <QStringList>
#include <QInputDialog>
#include <QtDebug>
#include <QXmppVCardIq.h>
#include <QXmppPresence.h>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <util/util.h>
#include <interfaces/iproxyobject.h>
#include <interfaces/azothutil.h>
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "glooxprotocol.h"
#include "vcarddialog.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "util.h"
#include "core.h"
#include "capsmanager.h"
#include "useractivity.h"
#include "usermood.h"
#include "usertune.h"
#include "userlocation.h"
#include "adhoccommandmanager.h"
#include "executecommanddialog.h"
#include "roomclentry.h"
#include "roomhandler.h"
#include "useravatardata.h"
#include "useravatarmetadata.h"
#include "capsdatabase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	EntryBase::EntryBase (GlooxAccount *parent)
	: QObject (parent)
	, Commands_ (0)
	, Account_ (parent)
	, HasUnreadMsgs_ (false)
	{
		connect (this,
				SIGNAL (locationChanged (const QString&, QObject*)),
				parent,
				SIGNAL (geolocationInfoChanged (const QString&, QObject*)));
	}

	QObject* EntryBase::GetObject ()
	{
		return this;
	}

	QList<QObject*> EntryBase::GetAllMessages () const
	{
		return AllMessages_;
	}

	void EntryBase::PurgeMessages (const QDateTime& before)
	{
		Azoth::Util::StandardPurgeMessages (AllMessages_, before);
	}

	namespace
	{
		bool CheckPartFeature (EntryBase *base, const QString& variant)
		{
			if (variant.isEmpty ())
				return true;

			const QByteArray& ver = base->GetVariantVerString (variant);
			if (ver.isEmpty ())
				return true;

			const QStringList& feats = Core::Instance ().GetCapsDatabase ()->Get (ver);
			if (feats.isEmpty ())
				return true;

			return feats.contains ("http://jabber.org/protocol/chatstates");
		}
	}

	void EntryBase::SetChatPartState (ChatPartState state, const QString& variant)
	{
		if (!CheckPartFeature (this, variant))
			return;

		// TODO check if participant supports this XEP.
		// Need to wait until disco info storage is implemented.
		QXmppMessage msg;
		msg.setTo (GetJID () + (variant.isEmpty () ?
						QString () :
						('/' + variant)));
		msg.setState (static_cast<QXmppMessage::State> (state));
		Account_->GetClientConnection ()->
				GetClient ()->sendPacket (msg);
	}

	EntryStatus EntryBase::GetStatus (const QString& variant) const
	{
		if (!variant.isEmpty () &&
				CurrentStatus_.contains (variant))
			return CurrentStatus_ [variant];

		if (CurrentStatus_.size ())
			return *CurrentStatus_.begin ();

		return EntryStatus ();
	}

	QList<QAction*> EntryBase::GetActions () const
	{
		QList<QAction*> additional;

		if (!Commands_)
		{
			Commands_ = new QAction (tr ("Commands..."), Account_);
			connect (Commands_,
					SIGNAL (triggered ()),
					this,
					SLOT (handleCommands ()));
		}
		additional << Commands_;

		return additional + Actions_;
	}

	QImage EntryBase::GetAvatar () const
	{
		return Avatar_;
	}

	QString EntryBase::GetRawInfo () const
	{
		return RawInfo_;
	}

	void EntryBase::ShowInfo ()
	{
		if (Account_->GetState ().State_ == SOffline)
		{
			Entity e = LeechCraft::Util::MakeNotification ("Azoth",
					tr ("Can't view info while offline"),
					PCritical_);
			Core::Instance ().SendEntity (e);

			return;
		}

		if (!VCardDialog_)
			VCardDialog_ = new VCardDialog (this);

		Account_->GetClientConnection ()->FetchVCard (GetJID (), VCardDialog_);
		VCardDialog_->show ();
	}

	QMap<QString, QVariant> EntryBase::GetClientInfo (const QString& var) const
	{
		return Variant2ClientInfo_ [var];
	}

	void EntryBase::MarkMsgsRead ()
	{
		HasUnreadMsgs_ = false;
		emit messagesAreRead ();
	}

	IAdvancedCLEntry::AdvancedFeatures EntryBase::GetAdvancedFeatures () const
	{
		return AFSupportsAttention;
	}

	void EntryBase::DrawAttention (const QString& text, const QString& variant)
	{
		const QString& to = variant.isEmpty () ?
				GetJID () :
				GetJID () + '/' + variant;
		QXmppMessage msg;
		msg.setBody (text);
		msg.setTo (to);
		msg.setType (QXmppMessage::Headline);
		msg.setAttention (true);
		Account_->GetClientConnection ()->GetClient ()->sendPacket (msg);
	}

	void EntryBase::HandleMessage (GlooxMessage *msg)
	{
		if (msg->GetMessageType () == IMessage::MTChatMessage)
			HasUnreadMsgs_ = true;

		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		IProxyObject *proxy = qobject_cast<IProxyObject*> (proto->GetProxyObject ());
		proxy->PreprocessMessage (msg);

		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void EntryBase::HandlePEPEvent (QString variant, PEPEventBase *event)
	{
		const QStringList& vars = Variants ();
		if (!vars.isEmpty () &&
				(!vars.contains (variant) || variant.isEmpty ()))
			variant = vars.first ();

		UserActivity *activity = dynamic_cast<UserActivity*> (event);
		if (activity)
		{
			if (activity->GetGeneral () == UserActivity::GeneralEmpty)
				Variant2ClientInfo_ [variant].remove ("user_activity");
			else
			{
				QMap<QString, QVariant> activityMap;
				activityMap ["general"] = activity->GetGeneralStr ();
				activityMap ["specific"] = activity->GetSpecificStr ();
				activityMap ["text"] = activity->GetText ();
				Variant2ClientInfo_ [variant] ["user_activity"] = activityMap;
			}

			emit activityChanged (variant);
			return;
		}

		UserMood *mood = dynamic_cast<UserMood*> (event);
		if (mood)
		{
			if (mood->GetMood () == UserMood::MoodEmpty)
				Variant2ClientInfo_ [variant].remove ("user_mood");
			else
			{
				QMap<QString, QVariant> moodMap;
				moodMap ["mood"] = mood->GetMoodStr ();
				moodMap ["text"] = mood->GetText ();
				Variant2ClientInfo_ [variant] ["user_mood"] = moodMap;
			}

			emit moodChanged (variant);
			return;
		}

		UserTune *tune = dynamic_cast<UserTune*> (event);
		if (tune)
		{
			if (tune->IsNull ())
				Variant2ClientInfo_ [variant].remove ("user_tune");
			else
			{
				QMap<QString, QVariant> tuneMap;
				tuneMap ["artist"] = tune->GetArtist ();
				tuneMap ["source"] = tune->GetSource ();
				tuneMap ["title"] = tune->GetTitle ();
				tuneMap ["track"] = tune->GetTrack ();
				tuneMap ["URI"] = tune->GetURI ();
				tuneMap ["length"] = tune->GetLength ();
				tuneMap ["rating"] = tune->GetRating ();
				Variant2ClientInfo_ [variant] ["user_tune"] = tuneMap;
			}

			emit tuneChanged (variant);
			return;
		}

		UserLocation *location = dynamic_cast<UserLocation*> (event);
		if (location)
		{
			Location_ [variant] = location->GetInfo ();
			emit locationChanged (variant, this);
			emit locationChanged (variant);
			return;
		}

		if (dynamic_cast<UserAvatarData*> (event) || dynamic_cast<UserAvatarMetadata*> (event))
			return;

		qWarning () << Q_FUNC_INFO
				<< "unhandled PEP event from"
				<< GetJID ()
				<< "resource"
				<< variant;
	}

	void EntryBase::HandleAttentionMessage (const QXmppMessage& msg)
	{
		QString jid;
		QString resource;
		ClientConnection::Split (msg.from (), &jid, &resource);

		emit attentionDrawn (msg.body (), resource);
	}

	void EntryBase::UpdateChatState (QXmppMessage::State state, const QString& variant)
	{
		emit chatPartStateChanged (static_cast<ChatPartState> (state), variant);

		if (state == QXmppMessage::Gone)
		{
			GlooxMessage *msg = new GlooxMessage (IMessage::MTEventMessage,
					IMessage::DIn,
					GetJID (),
					variant,
					Account_->GetClientConnection ().get ());
			msg->SetMessageSubType (IMessage::MSTParticipantEndedConversation);
			HandleMessage (msg);
		}
	}

	void EntryBase::SetStatus (const EntryStatus& status, const QString& variant)
	{
		const bool existed = CurrentStatus_.contains (variant);
		const bool wasOffline = existed ?
				CurrentStatus_ [variant].State_ == SOffline :
				false;
		if (existed &&
				status == CurrentStatus_ [variant])
			return;

		CurrentStatus_ [variant] = status;

		const QStringList& vars = Variants ();
		if ((!existed || wasOffline) && !vars.isEmpty ())
		{
			const QString& highest = vars.first ();
			if (Location_.contains (QString ()))
				Location_ [highest] = Location_.take (QString ());
			if (Variant2ClientInfo_.contains (QString ()))
			{
				const QMap<QString, QVariant>& info = Variant2ClientInfo_.take (QString ());
				QStringList toCopy;
				toCopy << "user_tune" << "user_mood" << "user_activity";
				Q_FOREACH (const QString& key, toCopy)
					if (info.contains (key))
						Variant2ClientInfo_ [highest] [key] = info [key];
			}
		}

		emit statusChanged (status, variant);

		if (!existed ||
				(existed && status.State_ == SOffline) ||
				wasOffline)
			emit availableVariantsChanged (vars);

		if (status.State_ != SOffline)
		{
			QXmppRosterManager& rm = Account_->
					GetClientConnection ()->GetClient ()->rosterManager ();
			const QMap<QString, QXmppPresence>& presences =
					rm.getAllPresencesForBareJid (GetJID ());
			if (presences.contains (variant))
			{
				const int p = presences.value (variant).status ().priority ();
				Variant2ClientInfo_ [variant] ["priority"] = p;
			}
		}

		GlooxMessage *message = 0;
		if (GetEntryType () == ETPrivateChat)
			message = new GlooxMessage (IMessage::MTStatusMessage,
					IMessage::DIn,
					qobject_cast<RoomCLEntry*> (GetParentCLEntry ())->
							GetRoomHandler ()->GetRoomJID (),
					GetEntryName (),
					Account_->GetClientConnection ().get ());
		else
			message = new GlooxMessage (IMessage::MTStatusMessage,
				IMessage::DIn,
				GetJID (),
				variant,
				Account_->GetClientConnection ().get ());
		message->SetMessageSubType (IMessage::MSTParticipantStatusChange);

		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		IProxyObject *proxy = qobject_cast<IProxyObject*> (proto->GetProxyObject ());
		const QString& state = proxy->StateToString (status.State_);

		const QString& nick = GetEntryName () + '/' + variant;
		message->setProperty ("Azoth/Nick", nick);
		message->setProperty ("Azoth/TargetState", state);
		message->setProperty ("Azoth/StatusText", status.StatusString_);

		QString msg = tr ("%1 is now %2 (%3)")
				.arg (nick)
				.arg (state)
				.arg (status.StatusString_);
		message->SetBody (msg);
		HandleMessage (message);
	}

	void EntryBase::SetAvatar (const QByteArray& data)
	{
		if (!data.size ())
			SetAvatar (QImage ());
		else
			SetAvatar (QImage::fromData (data));
	}

	void EntryBase::SetAvatar (const QImage& avatar)
	{
		Avatar_ = avatar;

		emit avatarChanged (Avatar_);
	}

	QXmppVCardIq EntryBase::GetVCard () const
	{
		return VCardIq_;
	}

	void EntryBase::SetVCard (const QXmppVCardIq& vcard)
	{
		VCardIq_ = vcard;

		QString text = FormatRawInfo (vcard);
		if (!text.isEmpty ())
			text = QString ("gloox VCard:\n") + text;
		SetRawInfo (text);

		if (!vcard.photo ().isEmpty ())
			SetAvatar (vcard.photo ());

		if (VCardDialog_)
			VCardDialog_->UpdateInfo (vcard);

		Core::Instance ().ScheduleSaveRoster (10000);
	}

	void EntryBase::SetRawInfo (const QString& rawinfo)
	{
		RawInfo_ = rawinfo;

		emit rawinfoChanged (RawInfo_);
	}

	bool EntryBase::HasUnreadMsgs () const
	{
		return HasUnreadMsgs_;
	}

	void EntryBase::SetClientInfo (const QString& variant,
			const QString& node, const QByteArray& ver)
	{
		QString type = XooxUtil::GetClientIDName (node);
		if (type.isEmpty ())
		{
			if (!node.isEmpty ())
				qWarning () << Q_FUNC_INFO
						<< "unknown client type for"
						<< node;
			type = "unknown";
		}
		Variant2ClientInfo_ [variant] ["client_type"] = type;

		QString name = XooxUtil::GetClientHRName (node);
		if (name.isEmpty ())
		{
			if (!node.isEmpty ())
				qWarning () << Q_FUNC_INFO
						<< "unknown client name for"
						<< node;
			name = "Unknown";
		}
		Variant2ClientInfo_ [variant] ["client_name"] = name;

		Variant2VerString_ [variant] = ver;

		QString reqJid = GetJID ();
		if (GetEntryType () == ETChat)
			reqJid = variant.isEmpty () ?
					QString () :
					reqJid + '/' + variant;

		if (!reqJid.isEmpty ())
			Account_->GetClientConnection ()->
					GetCapsManager ()->FetchCaps (reqJid, ver);
	}

	void EntryBase::SetClientInfo (const QString& variant, const QXmppPresence& pres)
	{
		SetClientInfo (variant, pres.capabilityNode (), pres.capabilityVer ());
	}

	GeolocationInfo_t EntryBase::GetGeolocationInfo (const QString& variant) const
	{
		return Location_ [variant];
	}

	QByteArray EntryBase::GetVariantVerString (const QString& var) const
	{
		return Variant2VerString_ [var];
	}

	QString EntryBase::FormatRawInfo (const QXmppVCardIq& vcard)
	{
		QString text;
		text += tr ("Name: %1")
				.arg (vcard.fullName ());
		text += "\n";

		if (vcard.nickName ().size ())
			text += tr ("Nickname: %1\n")
					.arg (vcard.nickName ());
		if (vcard.url ().size ())
			text += tr ("URL: %1\n")
					.arg (vcard.url ());
		if (vcard.birthday ().isValid ())
			text += tr ("Birthday: %1\n")
					.arg (vcard.birthday ().toString ());
		if (vcard.email ().size ())
			text += tr ("Email: %1\n")
					.arg (vcard.email ());

		if (vcard.photoType ().size ())
		{
			text += tr ("Photo:") + QString ("\ndata:%1;base64,%2\n")
						.arg (vcard.photoType ())
						.arg (vcard.photo ().constData ());
		}

		return text;
	}

	void EntryBase::handleCommands ()
	{
		QString jid = GetJID ();
		if (GetEntryType () != ETPrivateChat)
		{
			QStringList commandable;
			Q_FOREACH (const QString& var, Variant2VerString_.keys ())
			{
				const QStringList& caps = Account_->GetClientConnection ()->
						GetCapsManager ()->GetRawCaps (Variant2VerString_ [var]);
				if (caps.isEmpty () ||
					caps.contains (AdHocCommandManager::GetAdHocFeature ()))
					commandable << var;
			}

			if (commandable.isEmpty ())
				return;
			else if (commandable.size () == 1)
				jid += '/' + commandable.first ();
			else
			{
				bool ok = true;
				const QString& var = QInputDialog::getItem (0,
						tr ("Select resource"),
						tr ("Select resource for which to fetch the commands"),
						commandable,
						0,
						false,
						&ok);
				if (!ok || var.isEmpty ())
					return;

				jid += '/' + var;
			}
		}

		ExecuteCommandDialog *dia = new ExecuteCommandDialog (jid, Account_);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}
}
}
}
