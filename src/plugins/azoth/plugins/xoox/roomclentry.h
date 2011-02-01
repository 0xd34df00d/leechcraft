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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_ROOMCLENTRY_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QStringList>
#include <interfaces/iclentry.h>
#include <interfaces/imucentry.h>

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	class GlooxAccount;
	class RoomPublicMessage;
	class RoomHandler;

	class RoomCLEntry : public QObject
					  , public ICLEntry
					  , public IMUCEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Plugins::Azoth::Plugins::ICLEntry
						LeechCraft::Plugins::Azoth::Plugins::IMUCEntry);

		GlooxAccount *Account_;
		QList<QObject*> AllMessages_;
		RoomHandler *RH_;
	public:
		RoomCLEntry (RoomHandler*, GlooxAccount*);

		RoomHandler* GetRoomHandler () const;

		// ICLEntry
		QObject* GetObject ();
		QObject* GetParentAccount () const ;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QByteArray GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QObject*> GetAllMessages () const;
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;

		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		void Leave (const QString&);
		QString GetNick () const;
		void SetNick (const QString&);
		bool MayChangeAffiliation (QObject*, MUCAffiliation) const;
		bool MayChangeRole (QObject*, MUCRole) const;
		MUCAffiliation GetAffiliation (QObject*) const;
		void SetAffiliation (QObject*, MUCAffiliation, const QString&);
		MUCRole GetRole (QObject*) const;
		void SetRole (QObject*, MUCRole, const QString&);

		void HandleMessage (RoomPublicMessage*);
		void HandleNewParticipants (const QList<ICLEntry*>&);
		void HandleSubjectChanged (const QString&);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const Plugins::EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);

		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);

		void participantAffiliationChanged (QObject*, MUCAffiliation);
		void participantRoleChanged (QObject*, MUCRole);
	};
}
}
}
}
}

#endif
