/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H

#include <QObject>
#include <interfaces/iclentry.h>
#include <interfaces/imucentry.h>
#include "entrybase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcServerHandler;
	class IrcAccount;

	class IrcServerCLEntry : public EntryBase
							, public IMUCEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::IMUCEntry)

		IrcServerHandler *ISH_;
		IrcAccount *Account_;
	public:
		IrcServerCLEntry (IrcServerHandler*, IrcAccount*);

		IrcServerHandler* GetIrcServerHandler () const;
		IrcAccount* GetIrcAccount () const;

		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType, const QString&,
				const QString&);
		// IMUCEntry
		MUCFeatures GetMUCFeatures () const;
		QString GetMUCSubject () const;
		void SetMUCSubject (const QString&);
		QList<QObject*> GetParticipants ();
		void Join ();
		void Leave (const QString& msg = QString ());
		QString GetNick () const;
		void SetNick (const QString&);
		QString GetGroupName () const;
		QString GetRealID (QObject*) const;
		QVariantMap GetIdentifyingData () const;
	signals:
		void gotNewParticipants (const QList<QObject*>&);
		void mucSubjectChanged (const QString&);
		void nicknameConflict (const QString&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCSERVERCLENTRY_H
