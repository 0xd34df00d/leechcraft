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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_GLOOXCLENTRY_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QStringList>
#include <QXmppRosterIq.h>
#include <interfaces/iauthable.h>
#include "entrybase.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
class IAccount;

namespace Xoox
{
	class GlooxAccount;

	class GlooxCLEntry : public EntryBase
					   , public IAuthable
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Plugins::Azoth::Plugins::IAuthable);

		QString BareJID_;
	public:
		struct OfflineDataSource
		{
			QByteArray ID_;
			QString Name_;
			QStringList Groups_;
			AuthStatus AuthStatus_;
		};
		typedef boost::shared_ptr<OfflineDataSource> OfflineDataSource_ptr;
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
	public:
		GlooxCLEntry (const QString& bareJID, GlooxAccount*);
		GlooxCLEntry (OfflineDataSource_ptr, GlooxAccount*);

		OfflineDataSource_ptr ToOfflineDataSource () const;
		void Convert2ODS ();

		void UpdateRI (const QXmppRosterIq::Item&);
		QXmppRosterIq::Item GetRI () const;
		//QList<const gloox::Resource*> GetResourcesDesc () const;

		// ICLEntry
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		/** Entry ID for GlooxCLEntry is its jid.
		 */
		QByteArray GetEntryID () const;
		QStringList Groups () const;
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		// IAuthable
		AuthStatus GetAuthStatus () const;
		void RevokeAuth (const QString&);
		void Unsubscribe (const QString&);
		void RerequestAuth (const QString&);

		QString GetJID () const;
	};
}
}
}
}
}

#endif
