/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPARTICIPANTENTRY_H

#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QStringList>
#include <interfaces/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	
	class IrcAccount;
	class ChannelHandler;
	class ChannePublicMessage;
	class IrcMessage;
	
	class ChannelParticipantEntry : public QObject
									, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry);
		
		IrcAccount *Account_;
		QString NickName_;
		ChannelHandler *ChannelHandler_;
		QList<QObject*> AllMessages_;
	public:
		ChannelParticipantEntry (const QString&, ChannelHandler*, IrcAccount*);

		QObject* GetParentAccount () const ;
		QObject* GetParentCLEntry () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		void HandleMessage (IrcMessage*);
		QString GetChannelID () const;
		QString GetNick () const;
		
		QObject* GetObject ();
		QList<QObject*> GetAllMessages () const;
		EntryStatus GetStatus (const QString& variant = QString()) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
	signals:
		void availableVariantsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void gotMessage (QObject*);
		void groupsChanged (const QStringList&);
		void nameChanged (const QString&);
		void rawinfoChanged (const QString&);
		void statusChanged (const EntryStatus&, const QString&);
	};

	typedef boost::shared_ptr<ChannelParticipantEntry> ChannelParticipantEntry_ptr;
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_CHANNELPARTICIPANTENTRY_H
