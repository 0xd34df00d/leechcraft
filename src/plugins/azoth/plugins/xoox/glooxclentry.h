/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QXmppRosterIq.h>
#include <QXmppVCardIq.h>
#include <QXmppPresence.h>
#include <interfaces/azoth/iauthable.h>
#include "entrybase.h"
#include "offlinedatasource.h"

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
	private:
		OfflineDataSource_ptr ODS_;

		struct MessageQueueItem
		{
			IMessage::Type Type_;
			QString Variant_;
			QString Text_;
			QDateTime DateTime_;
		};
		QList<MessageQueueItem> MessageQueue_;

		bool AuthRequested_ = false;

		mutable QList<QAction*> GWActions_;
	public:
		GlooxCLEntry (const QString& bareJID, GlooxAccount*);
		GlooxCLEntry (OfflineDataSource_ptr, GlooxAccount*);

		OfflineDataSource_ptr ToOfflineDataSource () const;
		void Convert2ODS ();

		static QString JIDFromID (GlooxAccount*, const QString&);

		void UpdateRI (const QXmppRosterIq::Item&);
		QXmppRosterIq::Item GetRI () const;

		// ICLEntry
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		/** Entry ID for GlooxCLEntry is its jid.
		 */
		QString GetEntryID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		EntryStatus GetStatus (const QString&) const;
		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);
		QList<QAction*> GetActions () const;

		// IAuthable
		AuthStatus GetAuthStatus () const;
		void ResendAuth (const QString&);
		void RevokeAuth (const QString&);
		void Unsubscribe (const QString&);
		void RerequestAuth (const QString&);

		QString GetJID () const;

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
