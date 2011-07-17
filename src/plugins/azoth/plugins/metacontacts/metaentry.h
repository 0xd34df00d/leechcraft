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

#ifndef PLUGINS_AZOTH_PLUGINS_METACONTACTS_METAENTRY_H
#define PLUGINS_AZOTH_PLUGINS_METACONTACTS_METAENTRY_H
#include <QObject>
#include <QPair>
#include <QStringList>
#include <interfaces/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	class MetaAccount;

	class MetaEntry : public QObject
					, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry)
		
		MetaAccount *Account_;
		QString ID_;
		QString Name_;
		QStringList Groups_;
		
		QStringList UnavailableRealEntries_;
		QList<QObject*> AvailableRealEntries_;
		QMap<QString, QPair<QObject*, QString> > Variant2RealVariant_;
		
		QList<QObject*> Messages_;
	public:
		MetaEntry (const QString&, MetaAccount*);
		
		QStringList GetRealEntries () const;
		void SetRealEntries (const QStringList&);
		void AddRealObject (ICLEntry*);
		
		QString GetMetaVariant (QObject*, const QString&) const;
		
		QObject* GetObject ();
		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString& name);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		QObject* CreateMessage (IMessage::MessageType, const QString&, const QString&);
		QList<QObject*> GetAllMessages () const;
		void PurgeMessages (const QDateTime&);
		void SetChatPartState (ChatPartState, const QString&);
		EntryStatus GetStatus (const QString&) const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QList<QAction*> GetActions () const;
		QMap<QString, QVariant> GetClientInfo (const QString&) const;
	private slots:
		void handleGotMessage (QObject*);
		void handleRealVariantsChanged (QStringList, QObject* = 0);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void availableVariantsChanged (const QStringList&);
		void rawinfoChanged (const QString&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void avatarChanged (const QImage&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
		void permsChanged ();
	};
}
}
}

#endif
