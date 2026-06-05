/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#include <QObject>
#include <QStringList>
#include <QXmppRosterIq.h>
#include <QXmppPresence.h>
#include <interfaces/azoth/iauthable.h>
#include "entrybase.h"

namespace LC::Azoth
{
	class IAccount;
	class IProxyObject;
}

namespace LC::Azoth::Xoox
{
	class GlooxAccount;
	class PrivacyList;
	class VCardStorage;

	class GlooxCLEntry : public EntryBase
					   , public IAuthable
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IAuthable)

		struct MessageQueueItem
		{
			IMessage::Type Type_;
			QString Variant_;
			QString Text_;
			QDateTime DateTime_;
		};
		QList<MessageQueueItem> MessageQueue_;

		bool AuthRequested_ = false;
		bool IsRosterEntry_ = false;

		mutable QList<QAction*> GWActions_;
	public:
		GlooxCLEntry (const QString& bareJID, GlooxAccount*);

		static QString JIDFromID (GlooxAccount*, const QString&);

		bool IsRosterEntry () const;
		void PromoteToRosterEntry ();

		void UpdateRI (const QXmppRosterIq::Item&);
		QXmppRosterIq::Item GetRI () const;

		// ICLEntry
		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;
		QString GetEntryName () const override;
		void SetEntryName (const QString&) override;
		QString GetEntryID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList&) override;
		QStringList Variants () const override;
		EntryStatus GetStatus (const QString&) const override;
		void SendMessage (const OutgoingMessage& message) override;
		QList<QAction*> GetActions () const override;

		// IAuthable
		AuthStatus GetAuthStatus () const override;
		void ResendAuth (const QString&) override;
		void RevokeAuth (const QString&) override;
		void Unsubscribe (const QString&) override;
		void RerequestAuth (const QString&) override;

		QString GetJID () const override;

		bool IsGateway (QString* = 0) const;

		void SetAuthRequested (bool);
	private:
		void SendGWPresence (QXmppPresence::Type);
	private slots:
		void handleGWLogin ();
		void handleGWLogout ();
		void handleGWEdit ();
	};
}

#endif
