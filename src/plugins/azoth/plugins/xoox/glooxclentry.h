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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QXmppRosterIq.h>
#include <QXmppVCardIq.h>
#include <interfaces/iauthable.h>
#include "entrybase.h"

namespace LeechCraft
{
namespace Azoth
{
class IAccount;

namespace Xoox
{
	class GlooxAccount;

	struct OfflineDataSource
	{
		QString ID_;
		QString Name_;
		QStringList Groups_;
		AuthStatus AuthStatus_;
		QXmppVCardIq VCardIq_;
	};
	typedef std::shared_ptr<OfflineDataSource> OfflineDataSource_ptr;

	void Save (OfflineDataSource_ptr, QXmlStreamWriter*);
	void Load (OfflineDataSource_ptr, const QDomElement&);

	class GlooxCLEntry : public EntryBase
					   , public IAuthable
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IAuthable);

		QString BareJID_;
	private:
		OfflineDataSource_ptr ODS_;

		struct MessageQueueItem
		{
			IMessage::MessageType Type_;
			QString Variant_;
			QString Text_;
			QDateTime DateTime_;
		};
		QList<MessageQueueItem> MessageQueue_;

		bool AuthRequested_;

		mutable QAction *GWLogin_;
		mutable QAction *GWLogout_;
	public:
		GlooxCLEntry (const QString& bareJID, GlooxAccount*);
		GlooxCLEntry (OfflineDataSource_ptr, GlooxAccount*);

		OfflineDataSource_ptr ToOfflineDataSource () const;
		void Convert2ODS ();

		static QString JIDFromID (GlooxAccount*, const QString&);

		void UpdateRI (const QXmppRosterIq::Item&);
		QXmppRosterIq::Item GetRI () const;

		// ICLEntry
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		/** Entry ID for GlooxCLEntry is its jid.
		 */
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		EntryStatus GetStatus (const QString&) const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		virtual QList<QAction*> GetActions () const;

		// IAuthable
		AuthStatus GetAuthStatus () const;
		void ResendAuth (const QString&);
		void RevokeAuth (const QString&);
		void Unsubscribe (const QString&);
		void RerequestAuth (const QString&);

		QString GetJID () const;

		void SetAuthRequested (bool);
	private slots:
		void handleGWLogin ();
		void handleGWLogout ();
	};
}
}
}

#endif
