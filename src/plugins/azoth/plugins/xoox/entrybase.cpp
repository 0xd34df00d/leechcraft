/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entrybase.h"
#include <QImage>
#include <QStringList>
#include <QInputDialog>
#include <QtDebug>
#include <QBuffer>
#include <QTimer>
#include <QAction>
#include <QCryptographicHash>
#include <QXmppVCardIq.h>
#include <QXmppPresence.h>
#include <QXmppClient.h>
#include <QXmppRosterManager.h>
#include <QXmppDiscoveryManager.h>
#include <QXmppGlobal.h>
#include <QXmppEntityTimeIq.h>
#include <QXmppEntityTimeManager.h>
#include <QXmppVersionManager.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/azothutil.h>
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
#include "pepmicroblog.h"
#include "xeps/adhoccommandmanager.h"
#include "executecommanddialog.h"
#include "roomclentry.h"
#include "roomhandler.h"
#include "useravatardata.h"
#include "useravatarmetadata.h"
#include "capsdatabase.h"
#include "avatarsstorage.h"
#include "inforequestpolicymanager.h"
#include "xeps/pingmanager.h"
#include "pingreplyobject.h"
#include "pendingversionquery.h"
#include "discomanagerwrapper.h"
#include "vcardstorage.h"
#include "clientconnectionextensionsmanager.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	EntryBase::EntryBase (const QString& humanReadableId, GlooxAccount *parent)
	: QObject (parent)
	, Account_ (parent)
	, HumanReadableId_ (humanReadableId)
	, Commands_ (new QAction (tr ("Commands..."), this))
	, DetectNick_ (new QAction (tr ("Detect nick"), this))
	, StdSep_ (Util::CreateSeparator (this))
	, VCardPhotoHash_ (parent->GetParentProtocol ()->GetVCardStorage ()->
				GetVCardPhotoHash (humanReadableId).value_or (QByteArray {}))
	{
		connect (this,
				SIGNAL (locationChanged (const QString&, QObject*)),
				parent,
				SIGNAL (geolocationInfoChanged (const QString&, QObject*)));

		connect (Commands_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleCommands ()));
		connect (DetectNick_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleDetectNick ()));
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

	GlooxAccount* EntryBase::GetParentAccount () const
	{
		return Account_;
	}

	QList<IMessage*> EntryBase::GetAllMessages () const
	{
		QList<IMessage*> result;
		std::copy (AllMessages_.begin (), AllMessages_.end (), std::back_inserter (result));
		return result;
	}

	void EntryBase::PurgeMessages (const QDateTime& before)
	{
		AzothUtil::StandardPurgeMessages (AllMessages_, before);
	}

	namespace
	{
		bool CheckPartFeature (EntryBase *base, const QString& variant, CapsDatabase *capsDB)
		{
			return XooxUtil::CheckUserFeature (base,
					variant,
					"http://jabber.org/protocol/chatstates",
					capsDB);
		}
	}

	void EntryBase::SetChatPartState (ChatPartState state, const QString& variant)
	{
		if (!CheckPartFeature (this, variant, Account_->GetParentProtocol ()->GetCapsDatabase ()))
			return;

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
				Variants_.contains (variant))
			return Variants_ [variant].CurrentStatus_;

		if (!Variants_.isEmpty ())
			return Variants_.begin ()->CurrentStatus_;

		return EntryStatus ();
	}

	QList<QAction*> EntryBase::GetActions () const
	{
		QList<QAction*> additional;
		additional << Commands_;
		if (GetEntryFeatures () & FSupportsRenames)
			additional << DetectNick_;
		additional << StdSep_;

		return additional + Actions_;
	}

	QString EntryBase::GetHumanReadableID () const
	{
		return HumanReadableId_;
	}

	void EntryBase::ShowInfo ()
	{
		if (!VCardDialog_)
		{
			VCardDialog_ = new VCardDialog (this);
			VCardDialog_->setAttribute (Qt::WA_DeleteOnClose);
			VCardDialog_->UpdateInfo (GetVCard ());
		}
		VCardDialog_->show ();

		if (Account_->GetState ().State_ == SOffline)
		{
			Entity e = LC::Util::MakeNotification ("Azoth",
					tr ("Can't view info while offline"),
					Priority::Critical);
			Core::Instance ().SendEntity (e);

			return;
		}

		const auto& ptr = VCardDialog_;
		Account_->GetClientConnection ()->FetchVCard (GetJID (),
				[ptr] (const QXmppVCardIq& iq)
				{
					if (ptr)
						ptr->UpdateInfo (iq);
				},
				true);
	}

	QMap<QString, QVariant> EntryBase::GetClientInfo (const QString& var) const
	{
		const auto& varInfo = Variants_ [var];
		auto res = varInfo.ClientInfo_;

		if (varInfo.SecsDiff_)
		{
			auto now = QDateTime::currentDateTimeUtc ();
			now.setTimeSpec (Qt::LocalTime);
			const auto& secsDiff = *varInfo.SecsDiff_;
			res ["client_time"] = now
					.addSecs (secsDiff.Diff_)
					.addSecs (secsDiff.Tzo_);
			res ["client_tzo"] = secsDiff.Tzo_;
		}

		const auto& version = varInfo.Version_;
		if (version.name ().isEmpty ())
			return res;

		res ["client_remote_name"] = version.name ();
		if (!version.version ().isEmpty ())
			res ["client_version"] = version.version ();
		if (!version.os ().isEmpty ())
			res ["client_os"] = version.os ();
		if (res ["client_name"].toString ().isEmpty ())
			res ["client_name"] = version.name ();

		return res;
	}

	void EntryBase::MarkMsgsRead ()
	{
		HasUnreadMsgs_ = false;
		UnreadMessages_.clear ();

		Account_->GetParentProtocol ()->GetProxyObject ()->MarkMessagesAsRead (this);
	}

	void EntryBase::ChatTabClosed ()
	{
		emit chatTabClosed ();
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
		msg.setAttentionRequested (true);
		Account_->GetClientConnection ()->GetClient ()->sendPacket (msg);
	}

	QVariant EntryBase::GetMetaInfo (DataField field) const
	{
		switch (field)
		{
		case DataField::BirthDate:
			return GetVCard ().birthday ();
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown data field"
				<< static_cast<int> (field);

		return QVariant ();
	}

	QList<QPair<QString, QVariant>> EntryBase::GetVCardRepresentation () const
	{
		Account_->GetClientConnection ()->FetchVCard (GetJID ());

		const auto vcard = GetVCard ();

		QList<QPair<QString, QVariant>> result
		{
			{ tr ("Photo"), QImage::fromData (vcard.photo ()) },
			{ "JID", vcard.from () },
			{ tr ("Real name"), vcard.fullName () },
			{ tr ("Birthday"), vcard.birthday () },
			{ "URL", vcard.url () },
			{ tr ("About"), vcard.description () }
		};

		for (const auto& phone : vcard.phones ())
		{
			if (phone.number ().isEmpty ())
				continue;

			QStringList attrs;
			if (phone.type () & QXmppVCardPhone::Preferred)
				attrs << tr ("preferred");
			if (phone.type () & QXmppVCardPhone::Home)
				attrs << tr ("home");
			if (phone.type () & QXmppVCardPhone::Work)
				attrs << tr ("work");
			if (phone.type () & QXmppVCardPhone::Cell)
				attrs << tr ("cell");

			result.append ({ tr ("Phone"), attrs.isEmpty () ?
						phone.number () :
						phone.number () + " (" + attrs.join (", ") + ")" });
		}

		for (const auto& email : vcard.emails ())
		{
			if (email.address ().isEmpty ())
				continue;

			QStringList attrs;
			if (email.type () == QXmppVCardEmail::Preferred)
				attrs << tr ("preferred");
			if (email.type () == QXmppVCardEmail::Home)
				attrs << tr ("home");
			if (email.type () == QXmppVCardEmail::Work)
				attrs << tr ("work");
			if (email.type () == QXmppVCardEmail::X400)
				attrs << "X400";

			result.append ({ "Email", attrs.isEmpty () ?
						email.address () :
						email.address () + " (" + attrs.join (", ") + ")" });
		}

		for (const auto& address : vcard.addresses ())
		{
			if ((address.country () + address.locality () + address.postcode () +
					address.region () + address.street ()).isEmpty ())
				continue;

			QStringList attrs;
			if (address.type () & QXmppVCardAddress::Home)
				attrs << tr ("home");
			if (address.type () & QXmppVCardAddress::Work)
				attrs << tr ("work");
			if (address.type () & QXmppVCardAddress::Postal)
				attrs << tr ("postal");
			if (address.type () & QXmppVCardAddress::Preferred)
				attrs << tr ("preferred");

			QString str;
			QStringList fields;
			auto addField = [&fields] (const QString& label, const QString& val)
			{
				if (!val.isEmpty ())
					fields << label.arg (val);
			};
			addField (tr ("Country: %1"), address.country ());
			addField (tr ("Region: %1"), address.region ());
			addField (tr ("Locality: %1", "User's locality"), address.locality ());
			addField (tr ("Street: %1"), address.street ());
			addField (tr ("Postal code: %1"), address.postcode ());

			result.append ({ tr ("Address"), fields });
		}

		const auto& orgInfo = vcard.organization ();
		result.append ({ tr ("Organization"), orgInfo.organization () });
		result.append ({ tr ("Organization unit"), orgInfo.unit () });
		result.append ({ tr ("Job title"), orgInfo.title () });
		result.append ({ tr ("Job role"), orgInfo.role () });
		return result;
	}

	bool EntryBase::CanSendDirectedStatusNow (const QString& variant)
	{
		if (variant.isEmpty ())
			return true;

		if (GetStatus (variant).State_ != SOffline)
			return true;

		return false;
	}

	void EntryBase::SendDirectedStatus (const EntryStatus& state, const QString& variant)
	{
		if (!CanSendDirectedStatusNow (variant))
			return;

		auto conn = Account_->GetClientConnection ();

		auto pres = XooxUtil::StatusToPresence (state.State_,
				state.StatusString_, conn->GetLastState ().Priority_);

		QString to = GetJID ();
		if (!variant.isEmpty ())
			to += '/' + variant;
		pres.setTo (to);

		auto discoMgr = conn->GetClient ()->findExtension<QXmppDiscoveryManager> ();
		pres.setCapabilityHash ("sha-1");
		pres.setCapabilityNode (discoMgr->clientCapabilitiesNode ());
		pres.setCapabilityVer (discoMgr->capabilities ().verificationString ());
		conn->GetClient ()->sendPacket (pres);
	}

	void EntryBase::RequestLastPosts (int)
	{
	}

	namespace
	{
		QByteArray ComputeVCardPhotoHash (const QXmppVCardIq& vcard)
		{
			const auto& photo = vcard.photo ();
			return photo.isEmpty () ?
					QByteArray {} :
					QCryptographicHash::hash (photo, QCryptographicHash::Sha1);
		}
	}

	QFuture<QImage> EntryBase::RefreshAvatar (Size)
	{
		const auto maybeVCard = Account_->GetParentProtocol ()->
				GetVCardStorage ()->GetVCard (GetHumanReadableID ());
		if (maybeVCard && VCardPhotoHash_ == ComputeVCardPhotoHash (*maybeVCard))
			return Util::MakeReadyFuture (QImage::fromData (maybeVCard->photo ()));

		if (!Account_->GetClientConnection ()->IsConnected ())
			return Util::MakeReadyFuture (QImage {});

		QFutureInterface<QImage> iface;
		iface.reportStarted ();

		const auto cancelTimer = new QTimer;
		cancelTimer->setSingleShot (true);
		cancelTimer->setTimerType (Qt::VeryCoarseTimer);
		cancelTimer->start (120 * 1000);
		connect (cancelTimer,
				&QTimer::timeout,
				[iface, cancelTimer] () mutable
				{
					if (!iface.isFinished ())
						Util::ReportFutureResult (iface, QImage {});
					cancelTimer->deleteLater ();
				});

		Account_->GetClientConnection ()->FetchVCard (GetJID (),
				[iface, cancelTimer] (const QXmppVCardIq& iq) mutable
				{
					if (iface.isFinished ())
						return;

					const auto& photo = iq.photo ();
					const auto image = photo.isEmpty () ?
							QImage {} :
							QImage::fromData (photo);
					iface.reportFinished (&image);
					delete cancelTimer;
				});

		return iface.future ();
	}

	bool EntryBase::HasAvatar () const
	{
		return !VCardPhotoHash_.isEmpty ();
	}

	bool EntryBase::SupportsSize (Size size) const
	{
		return size == Size::Full;
	}

	Media::AudioInfo EntryBase::GetUserTune (const QString& variant) const
	{
		return Variants_ [GetVariantOrHighest (variant)].Audio_.value_or (Media::AudioInfo {});
	}

	MoodInfo EntryBase::GetUserMood (const QString& variant) const
	{
		return Variants_ [GetVariantOrHighest (variant)].Mood_.value_or (MoodInfo {});
	}

	ActivityInfo EntryBase::GetUserActivity (const QString& variant) const
	{
		return Variants_ [GetVariantOrHighest (variant)].Activity_.value_or (ActivityInfo {});
	}

	void EntryBase::UpdateEntityTime ()
	{
		const auto& now = QDateTime::currentDateTime ();
		if (LastEntityTimeRequest_.isValid () &&
				LastEntityTimeRequest_.secsTo (now) < 60)
			return;

		auto& timeMgr = Account_->GetClientConnection ()->Exts ().Get<QXmppEntityTimeManager> ();
		connect (&timeMgr,
				SIGNAL (timeReceived (QXmppEntityTimeIq)),
				this,
				SLOT (handleTimeReceived (QXmppEntityTimeIq)),
				Qt::UniqueConnection);

		LastEntityTimeRequest_ = now;

		auto jid = GetJID ();

		if (jid.contains ('/'))
		{
			timeMgr.requestTime (jid);
			return;
		}

		for (const auto& variant : Variants ())
			if (!variant.isEmpty ())
				timeMgr.requestTime (jid + '/' + variant);
	}

	QObject* EntryBase::Ping (const QString& variant)
	{
		auto jid = GetJID ();
		if (!variant.isEmpty ())
			jid += '/' + variant;

		auto reply = new PingReplyObject { this };
		Account_->GetClientConnection ()->Exts ().Get<PingManager> ().Ping (jid,
				[reply] (int msecs) { reply->HandleReply (msecs); });
		return reply;
	}

	QObject* EntryBase::QueryVersion (const QString& variant)
	{
		auto jid = GetJID ();
		if (!variant.isEmpty ())
			jid += '/' + variant;

		const auto vm = Account_->GetClientConnection ()->GetVersionManager ();
		vm->requestVersion (jid);

		return new PendingVersionQuery { vm, jid, this };
	}

	const QByteArray& EntryBase::GetVCardPhotoHash () const
	{
		return VCardPhotoHash_;
	}

	void EntryBase::HandlePresence (const QXmppPresence& pres, const QString& resource)
	{
		SetClientInfo (resource, pres);
		SetStatus (XooxUtil::PresenceToStatus (pres), resource, pres);

		CheckVCardUpdate (pres);
	}

	void EntryBase::HandleMessage (GlooxMessage *msg)
	{
		if (msg->GetMessageType () == IMessage::Type::ChatMessage)
		{
			HasUnreadMsgs_ = true;
			UnreadMessages_ << msg;
		}

		const auto proxy = Account_->GetParentProtocol ()->GetProxyObject ();
		proxy->GetFormatterProxy ().PreprocessMessage (msg);

		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void EntryBase::HandlePEPEvent (QString variant, PEPEventBase *event)
	{
		const auto& vars = Variants ();
		if (!vars.isEmpty () &&
				(!vars.contains (variant) || variant.isEmpty ()))
			variant = vars.first ();

		if (const auto activity = dynamic_cast<UserActivity*> (event))
			return HandleUserActivity (activity, variant);
		if (const auto mood = dynamic_cast<UserMood*> (event))
			return HandleUserMood (mood, variant);
		if (const auto tune = dynamic_cast<UserTune*> (event))
			return HandleUserTune (tune, variant);

		if (const auto location = dynamic_cast<UserLocation*> (event))
		{
			auto& currentLocation = Variants_ [variant].Location_;
			if (currentLocation == location->GetInfo ())
				return;

			currentLocation = location->GetInfo ();
			emit locationChanged (variant, this);
			emit locationChanged (variant);
			return;
		}

		if (PEPMicroblog *microblog = dynamic_cast<PEPMicroblog*> (event))
		{
			emit gotNewPost (*microblog);
			return;
		}

		if (dynamic_cast<UserAvatarData*> (event) || dynamic_cast<UserAvatarMetadata*> (event))
			return;

		qWarning () << Q_FUNC_INFO
				<< "unhandled PEP event from"
				<< GetJID ();
	}

	void EntryBase::HandleAttentionMessage (const QXmppMessage& msg)
	{
		auto resource = ClientConnection::Split (msg.from ()).Resource_;
		emit attentionDrawn (msg.body (), resource);
	}

	void EntryBase::UpdateChatState (QXmppMessage::State state, const QString& variant)
	{
		emit chatPartStateChanged (static_cast<ChatPartState> (state), variant);

		if (state == QXmppMessage::Gone)
		{
			const auto msg = new GlooxMessage (IMessage::Type::EventMessage,
					IMessage::Direction::In,
					GetJID (),
					variant,
					Account_->GetClientConnection ().get ());
			msg->SetMessageSubType (IMessage::SubType::ParticipantEndedConversation);
			HandleMessage (msg);
		}
	}

	void EntryBase::SetErrorPresence (const QString& variant, const QXmppPresence& presence)
	{
		if (!variant.isEmpty ())
		{
			if (Variants_.contains (variant))
				SetStatus ({}, variant, {});
			return;
		}

		for (const auto& var : Variants ())
			SetStatus ({}, var, {});

		SetStatus ({ SError, presence.error ().text () }, {}, {});
	}

	void EntryBase::SetStatus (const EntryStatus& status, const QString& variant, const QXmppPresence& presence)
	{
		const bool existed = Variants_.contains (variant);
		auto& varInfo = Variants_ [variant];
		const bool wasOffline = existed ?
				varInfo.CurrentStatus_.State_ == SOffline :
				false;

		if (existed &&
				status == varInfo.CurrentStatus_ &&
				presence.priority () == varInfo.ClientInfo_.value ("priority"))
			return;

		varInfo.CurrentStatus_ = status;

		if ((!existed || wasOffline) &&
				status.State_ != SOffline)
		{
			auto conn = Account_->GetClientConnection ();
			if (conn->GetInfoReqPolicyManager ()->IsRequestAllowed (InfoRequest::Version, this))
			{
				if (!variant.isEmpty ())
					conn->FetchVersion (GetJID () + '/' + variant);
				else
					conn->FetchVersion (GetJID ());
			}
		}

		if (status.State_ != SOffline)
		{
			if (const int p = presence.priority ())
				varInfo.ClientInfo_ ["priority"] = p;
		}
		else
			Variants_.remove (variant);

		emit statusChanged (status, variant);

		if (!existed ||
				status.State_ == SOffline ||
				wasOffline)
			emit availableVariantsChanged (Variants ());

		GlooxMessage *message = 0;
		if (GetEntryType () == EntryType::PrivateChat)
			message = new GlooxMessage (IMessage::Type::StatusMessage,
					IMessage::Direction::In,
					qobject_cast<RoomCLEntry*> (GetParentCLEntryObject ())->
							GetRoomHandler ()->GetRoomJID (),
					GetEntryName (),
					Account_->GetClientConnection ().get ());
		else
			message = new GlooxMessage (IMessage::Type::StatusMessage,
				IMessage::Direction::In,
				GetJID (),
				variant,
				Account_->GetClientConnection ().get ());
		message->SetMessageSubType (IMessage::SubType::ParticipantStatusChange);

		const auto proxy = Account_->GetParentProtocol ()->GetProxyObject ();
		const auto& state = proxy->StateToString (status.State_);

		const auto& nick = GetEntryName () + '/' + variant;
		message->setProperty ("Azoth/Nick", nick);
		message->setProperty ("Azoth/TargetState", state);
		message->setProperty ("Azoth/StatusText", status.StatusString_);

		const auto& msg = tr ("%1 is now %2 (%3)")
				.arg (nick)
				.arg (state)
				.arg (status.StatusString_);
		message->SetBody (msg);
		HandleMessage (message);
	}

	QXmppVCardIq EntryBase::GetVCard () const
	{
		const auto storage = Account_->GetParentProtocol ()->GetVCardStorage ();
		return storage->GetVCard (GetHumanReadableID ()).value_or (QXmppVCardIq {});
	}

	void EntryBase::SetVCard (const QXmppVCardIq& vcard)
	{
		if (VCardDialog_)
			VCardDialog_->UpdateInfo (vcard);

		Account_->GetParentProtocol ()->GetVCardStorage ()->SetVCard (GetHumanReadableID (), vcard);

		emit vcardUpdated ();

		const auto& newPhotoHash = ComputeVCardPhotoHash (vcard);
		if (newPhotoHash != VCardPhotoHash_)
		{
			VCardPhotoHash_ = newPhotoHash;
			WriteDownPhotoHash ();
			emit avatarChanged (this);
		}
	}

	bool EntryBase::HasUnreadMsgs () const
	{
		return HasUnreadMsgs_;
	}

	QList<GlooxMessage*> EntryBase::GetUnreadMessages () const
	{
		return UnreadMessages_;
	}

	void EntryBase::SetClientInfo (const QString& variant,
			const QString& node, const QByteArray& ver)
	{
		auto& varInfo = Variants_ [variant];
		if (varInfo.VerString_ == ver)
			return;

		const auto& staticClientInfo = XooxUtil::GetStaticClientInfo (node);
		if (staticClientInfo.IsEmpty () && !node.isEmpty ())
			qWarning () << Q_FUNC_INFO
					<< "unknown client for"
					<< node;

		varInfo.ClientInfo_ ["client_type"] = staticClientInfo.ID_;
		varInfo.ClientInfo_ ["client_name"] = staticClientInfo.HumanReadableName_;
		varInfo.ClientInfo_ ["raw_client_name"] = staticClientInfo.HumanReadableName_;

		varInfo.VerString_ = ver;

		QString reqJid = GetJID ();
		QString reqVar = "";
		if (GetEntryType () == EntryType::Chat)
		{
			reqJid = variant.isEmpty () ?
					reqJid :
					reqJid + '/' + variant;
			reqVar = variant;
		}

		auto capsManager = Account_->GetClientConnection ()->GetCapsManager ();
		const auto& storedIds = capsManager->GetIdentities (ver);

		if (!storedIds.isEmpty ())
			SetDiscoIdentities (reqVar, storedIds);
		else
		{
			qDebug () << "requesting ids for" << reqJid << reqVar;
			QPointer<EntryBase> pThis (this);
			QPointer<CapsManager> pCM (capsManager);
			Account_->GetClientConnection ()->GetDiscoManagerWrapper ()->RequestInfo (reqJid,
				[ver, reqVar, pThis, pCM] (const QXmppDiscoveryIq& iq)
				{
					if (!ver.isEmpty () && pCM)
						pCM->SetIdentities (ver, iq.identities ());
					if (pThis)
						pThis->SetDiscoIdentities (reqVar, iq.identities ());
				});
		}
	}

	void EntryBase::SetClientInfo (const QString& variant, const QXmppPresence& pres)
	{
		if (pres.type () == QXmppPresence::Available)
			SetClientInfo (variant, pres.capabilityNode (), pres.capabilityVer ());
	}

	void EntryBase::SetClientVersion (const QString& variant, const QXmppVersionIq& version)
	{
		qDebug () << Q_FUNC_INFO << variant << version.os ();
		Variants_ [variant].Version_ = version;

		emit entryGenerallyChanged ();
	}

	void EntryBase::SetDiscoIdentities (const QString& variant, const QList<QXmppDiscoveryIq::Identity>& ids)
	{
		auto& varInfo = Variants_ [variant];
		varInfo.Identities_ = ids;

		const QString& name = ids.value (0).name ();
		const QString& type = ids.value (0).type ();
		if (name.contains ("Kopete"))
		{
			varInfo.ClientInfo_ ["client_type"] = "kopete";
			varInfo.ClientInfo_ ["client_name"] = "Kopete";
			varInfo.ClientInfo_ ["raw_client_name"] = "kopete";
			emit statusChanged (GetStatus (variant), variant);
		}
		else if (name.contains ("emacs", Qt::CaseInsensitive) ||
				name.contains ("jabber.el", Qt::CaseInsensitive))
		{
			varInfo.ClientInfo_ ["client_type"] = "jabber.el";
			varInfo.ClientInfo_ ["client_name"] = "Emacs Jabber.El";
			varInfo.ClientInfo_ ["raw_client_name"] = "jabber.el";
			emit statusChanged (GetStatus (variant), variant);
		}
		else if (type == "mrim")
		{
			varInfo.ClientInfo_ ["client_type"] = "mailruagent";
			varInfo.ClientInfo_ ["client_name"] = "Mail.Ru Agent Gateway";
			varInfo.ClientInfo_ ["raw_client_name"] = "mailruagent";
			emit statusChanged (GetStatus (variant), variant);
		}
	}

	GeolocationInfo_t EntryBase::GetGeolocationInfo (const QString& variant) const
	{
		return Variants_ [variant].Location_;
	}

	QByteArray EntryBase::GetVariantVerString (const QString& var) const
	{
		return Variants_ [var].VerString_;
	}

	QXmppVersionIq EntryBase::GetClientVersion (const QString& var) const
	{
		return Variants_ [var].Version_;
	}

	void EntryBase::HandleUserActivity (const UserActivity *activity, const QString& variant)
	{
		auto& varInfo = Variants_ [variant];
		if (activity->GetGeneral () == UserActivity::GeneralEmpty)
		{
			if (!varInfo.Activity_)
				return;

			varInfo.Activity_.reset ();
		}
		else
		{
			const ActivityInfo info
			{
				activity->GetGeneralStr (),
				activity->GetSpecificStr (),
				activity->GetText ()
			};
			if (varInfo.Activity_ == info)
				return;

			varInfo.Activity_ = info;
		}

		emit activityChanged (variant);
	}

	void EntryBase::HandleUserMood (const UserMood *mood, const QString& variant)
	{
		auto& varInfo = Variants_ [variant];
		if (mood->GetMood () == UserMood::MoodEmpty)
		{
			if (!varInfo.Mood_)
				return;

			varInfo.Mood_.reset ();
		}
		else
		{
			const MoodInfo info
			{
				mood->GetMoodStr (),
				mood->GetText ()
			};
			if (varInfo.Mood_ == info)
				return;

			varInfo.Mood_ = info;
		}

		emit moodChanged (variant);
	}

	void EntryBase::HandleUserTune (const UserTune *tune, const QString& variant)
	{
		auto& varInfo = Variants_ [variant];
		if (tune->IsNull ())
		{
			if (!varInfo.Audio_)
				return;

			varInfo.Audio_.reset ();
		}
		else
		{
			const auto& audioInfo = tune->ToAudioInfo ();
			if (varInfo.Audio_ == audioInfo)
				return;

			varInfo.Audio_ = audioInfo;
		}

		emit tuneChanged (variant);
	}

	void EntryBase::CheckVCardUpdate (const QXmppPresence& pres)
	{
		auto conn = Account_->GetClientConnection ();
		if (!conn->GetInfoReqPolicyManager ()->IsRequestAllowed (InfoRequest::VCard, this))
			return;

		const auto& vcardUpdate = pres.vCardUpdateType ();
		if (vcardUpdate == QXmppPresence::VCardUpdateNoPhoto)
		{
			if (!VCardPhotoHash_.isEmpty ())
			{
				VCardPhotoHash_.clear ();
				WriteDownPhotoHash ();
				emit avatarChanged (this);
			}
		}
		else if (vcardUpdate == QXmppPresence::VCardUpdateValidPhoto)
		{
			if (pres.photoHash () != VCardPhotoHash_)
			{
				VCardPhotoHash_ = pres.photoHash ();
				WriteDownPhotoHash ();
				emit avatarChanged (this);
			}
		}
	}

	void EntryBase::SetNickFromVCard (const QXmppVCardIq& vcard)
	{
		if (!vcard.nickName ().isEmpty ())
		{
			SetEntryName (vcard.nickName ());
			return;
		}

		if (!vcard.fullName ().isEmpty ())
		{
			SetEntryName (vcard.fullName ());
			return;
		}

		const QString& fn = vcard.firstName ();
		const QString& mn = vcard.middleName ();
		const QString& ln = vcard.lastName ();
		QString name = fn;
		if (!fn.isEmpty ())
			name += " ";
		name += mn;
		if (!mn.isEmpty ())
			name += " ";
		name += ln;
		name = name.trimmed ();
		if (!name.isEmpty ())
			SetEntryName (name);
	}

	void EntryBase::WriteDownPhotoHash () const
	{
		const auto vcardStorage = Account_->GetParentProtocol ()->GetVCardStorage ();
		vcardStorage->SetVCardPhotoHash (GetHumanReadableID (), VCardPhotoHash_);
	}

	QString EntryBase::GetVariantOrHighest (const QString& var) const
	{
		return var.isEmpty () ?
				Variants ().value (0) :
				var;
	}

	void EntryBase::handleTimeReceived (const QXmppEntityTimeIq& iq)
	{
		const auto& from = iq.from ();
		if (!from.startsWith (GetJID ()))
			return;

		const auto& thatTime = iq.utc ();
		if (!thatTime.isValid ())
			return;

		auto [bare, variant] = ClientConnection::Split (from);

		if (variant.isEmpty () || GetEntryType () == EntryType::PrivateChat)
			variant = "";

		const auto secsDiff = QDateTime::currentDateTimeUtc ().secsTo (thatTime);
		Variants_ [variant].SecsDiff_ = { static_cast<int> (secsDiff), iq.tzo () };

		emit entryGenerallyChanged ();

		emit entityTimeUpdated ();
	}

	void EntryBase::handleCommands ()
	{
		auto jid = GetJID ();
		if (GetEntryType () != EntryType::PrivateChat)
		{
			QStringList commandable;
			const auto capsMgr = Account_->GetClientConnection ()->GetCapsManager ();
			for (const auto& pair : Util::Stlize (Variants_))
			{
				const auto& caps = capsMgr->GetRawCaps (pair.second.VerString_);
				if (caps.isEmpty () ||
						caps.contains (AdHocCommandManager::GetAdHocFeature ()))
					commandable << pair.first;
			}

			if (commandable.isEmpty ())
				return;
			else if (commandable.size () == 1)
			{
				const auto& var = commandable.first ();
				if (!var.isEmpty ())
					jid += '/' + var;
			}
			else
			{
				bool ok = true;
				const auto& var = QInputDialog::getItem (0,
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

		const auto dia = new ExecuteCommandDialog (jid, Account_);
		dia->show ();
	}

	void EntryBase::handleDetectNick ()
	{
		QPointer<EntryBase> ptr (this);
		Account_->GetClientConnection ()->FetchVCard (GetJID (),
				[ptr] (const QXmppVCardIq& iq) { if (ptr) ptr->SetNickFromVCard (iq); });
	}
}
}
}
