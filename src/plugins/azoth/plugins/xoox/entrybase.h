/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QImage>
#include <QMap>
#include <QVariant>
#include <QXmppMessage.h>
#include <QXmppVCardIq.h>
#include <QXmppVersionIq.h>
#include <QXmppDiscoveryIq.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iadvancedclentry.h>
#include <interfaces/azoth/imetainfoentry.h>
#include <interfaces/azoth/ihavedirectedstatus.h>
#include <interfaces/azoth/isupportgeolocation.h>
#include <interfaces/azoth/isupportmicroblogs.h>
#include <interfaces/azoth/ihaveentitytime.h>
#include <interfaces/azoth/ihavepings.h>
#include <interfaces/azoth/ihavequeriableversion.h>
#include <interfaces/azoth/ihavecontacttune.h>
#include <interfaces/azoth/ihavecontactmood.h>
#include <interfaces/azoth/ihavecontactactivity.h>
#include <interfaces/azoth/ihaveavatars.h>
#include <interfaces/azoth/moodinfo.h>
#include <interfaces/azoth/activityinfo.h>
#include "glooxaccount.h"

class QXmppPresence;
class QXmppVersionIq;
class QXmppEntityTimeIq;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class ClientConnection;
	class PEPEventBase;
	class GlooxMessage;
	class VCardDialog;
	class GlooxAccount;
	class UserTune;
	class UserMood;
	class UserActivity;

	/** Common base class for GlooxCLEntry, which reprensents usual
	 * entries in the contact list, and RoomCLEntry, which represents
	 * participants in MUCs.
	 *
	 * This class tries to unify and provide a common implementation of
	 * what those classes, well, have in common.
	 */
	class EntryBase : public QObject
					, public ICLEntry
					, public IAdvancedCLEntry
					, public IMetaInfoEntry
					, public IHaveDirectedStatus
					, public ISupportMicroblogs
					, public IHaveAvatars
					, public IHaveContactTune
					, public IHaveContactMood
					, public IHaveContactActivity
					, public IHaveEntityTime
					, public IHavePings
					, public IHaveQueriableVersion
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ICLEntry
				LC::Azoth::IAdvancedCLEntry
				LC::Azoth::IMetaInfoEntry
				LC::Azoth::IHaveDirectedStatus
				LC::Azoth::ISupportMicroblogs
				LC::Azoth::IHaveAvatars
				LC::Azoth::IHaveContactTune
				LC::Azoth::IHaveContactMood
				LC::Azoth::IHaveContactActivity
				LC::Azoth::IHaveEntityTime
				LC::Azoth::IHavePings
				LC::Azoth::IHaveQueriableVersion)
	protected:
		GlooxAccount *Account_;

		const QString HumanReadableId_;

		QList<GlooxMessage*> AllMessages_;
		QList<GlooxMessage*> UnreadMessages_;
		QList<QAction*> Actions_;
		QAction *Commands_;
		QAction *DetectNick_;
		QAction *StdSep_;

		struct EntityTimeInfo
		{
			int Diff_;
			int Tzo_;
		};

		struct VariantInfo
		{
			EntryStatus CurrentStatus_;
			GeolocationInfo_t Location_;
			QMap<QString, QVariant> ClientInfo_;
			QByteArray VerString_;
			QXmppVersionIq Version_;
			QList<QXmppDiscoveryIq::Identity> Identities_;
			std::optional<Media::AudioInfo> Audio_;
			std::optional<MoodInfo> Mood_;
			std::optional<ActivityInfo> Activity_;
			std::optional<EntityTimeInfo> SecsDiff_;
		};
		QHash<QString, VariantInfo> Variants_;

		QPointer<VCardDialog> VCardDialog_;

		QByteArray VCardPhotoHash_;

		QDateTime LastEntityTimeRequest_;

		bool HasUnreadMsgs_ = false;
	public:
		EntryBase (const QString& humanReadableId, GlooxAccount* = nullptr);
		virtual ~EntryBase ();

		// ICLEntry
		QObject* GetQObject ();
		GlooxAccount* GetParentAccount () const;
		QList<IMessage*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		QString GetHumanReadableID () const final;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
		void MarkMsgsRead ();
		void ChatTabClosed ();

		// IAdvancedCLEntry
		AdvancedFeatures GetAdvancedFeatures () const;
		void DrawAttention (const QString&, const QString&);

		// IMetaInfoEntry
		QVariant GetMetaInfo (DataField) const;
		QList<QPair<QString, QVariant>> GetVCardRepresentation () const;

		// IHaveDirectedStatus
		bool CanSendDirectedStatusNow (const QString&);
		void SendDirectedStatus (const EntryStatus&, const QString&);

		// ISupportMicroblogs
		void RequestLastPosts (int);

		// IHaveAvatars
		QFuture<QImage> RefreshAvatar (Size);
		bool HasAvatar () const;
		bool SupportsSize (Size) const;

		// IHaveContactTune
		Media::AudioInfo GetUserTune (const QString&) const;

		// IHaveContactMood
		MoodInfo GetUserMood (const QString&) const;

		// IHaveContactActivity
		ActivityInfo GetUserActivity (const QString&) const;

		// IHaveEntityTime
		void UpdateEntityTime ();

		// IHavePings
		QObject* Ping (const QString& variant);

		// IHaveQueriableVersion
		QObject* QueryVersion (const QString& variant);

		const QByteArray& GetVCardPhotoHash () const;

		virtual QString GetJID () const = 0;

		virtual void HandlePresence (const QXmppPresence&, const QString&);

		void HandleMessage (GlooxMessage*);
		void HandlePEPEvent (QString, PEPEventBase*);
		void HandleAttentionMessage (const QXmppMessage&);
		void UpdateChatState (QXmppMessage::State, const QString&);
		void SetErrorPresence (const QString&, const QXmppPresence&);
		void SetStatus (const EntryStatus&, const QString&, const QXmppPresence&);
		QXmppVCardIq GetVCard () const;
		void SetVCard (const QXmppVCardIq&);

		bool HasUnreadMsgs () const;
		QList<GlooxMessage*> GetUnreadMessages () const;

		void SetClientInfo (const QString&, const QString&, const QByteArray&);
		void SetClientInfo (const QString&, const QXmppPresence&);
		void SetClientVersion (const QString&, const QXmppVersionIq&);
		void SetDiscoIdentities (const QString&, const QList<QXmppDiscoveryIq::Identity>&);

		GeolocationInfo_t GetGeolocationInfo (const QString&) const;

		QByteArray GetVariantVerString (const QString&) const;
		QXmppVersionIq GetClientVersion (const QString&) const;
	private:
		void HandleUserActivity (const UserActivity*, const QString&);
		void HandleUserMood (const UserMood*, const QString&);
		void HandleUserTune (const UserTune*, const QString&);

		void CheckVCardUpdate (const QXmppPresence&);
		void SetNickFromVCard (const QXmppVCardIq&);

		void WriteDownPhotoHash () const;

		QString GetVariantOrHighest (const QString&) const;
	private slots:
		void handleTimeReceived (const QXmppEntityTimeIq&);

		void handleCommands ();
		void handleDetectNick ();
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
		void entryGenerallyChanged ();

		void chatTabClosed ();

		void attentionDrawn (const QString&, const QString&);
		void moodChanged (const QString&);
		void activityChanged (const QString&);
		void tuneChanged (const QString&);
		void locationChanged (const QString&);

		void gotRecentPosts (const QList<LC::Azoth::Post>&);
		void gotNewPost (const LC::Azoth::Post&);

		void locationChanged (const QString&, QObject*);

		void vcardUpdated ();

		void entityTimeUpdated ();

		void avatarChanged (QObject*);
	};
}
}
}
