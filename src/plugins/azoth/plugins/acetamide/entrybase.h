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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ENTRYBASE_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ENTRYBASE_H

#include <QObject>
#include <QImage>
#include <QVariant>
#include <interfaces/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;
	class IrcMessage;

	class EntryBase : public QObject
						, public ICLEntry
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Azoth::ICLEntry)
	protected:
		QList<QObject*> AllMessages_;
		EntryStatus CurrentStatus_;
		QList<QAction*> Actions_;

		IrcAccount *Account_;
	public:
		EntryBase (IrcAccount* = 0);

		virtual QObject* GetObject ();
		virtual QList<QObject*> GetAllMessages () const;
		EntryStatus GetStatus (const QString&) const;
		QList<QAction*> GetActions () const;
		QImage GetAvatar () const;
		QString GetRawInfo () const;
		void ShowInfo ();
		QMap<QString, QVariant> GetClientInfo (const QString&) const;

		virtual QString GetEntryID () const = 0;

		void HandleMessage (IrcMessage*);
		void SetStatus (const EntryStatus&);
		void SetAvatar (const QByteArray&);
		void SetAvatar (const QImage&);
		void SetRawInfo (const QString&);
	signals:
		void gotMessage (QObject*);
		void statusChanged (const EntryStatus&, const QString&);
		void avatarChanged (const QImage&);
		void rawinfoChanged (const QString&);
		void availableVariantsChanged (const QStringList&);
		void nameChanged (const QString&);
		void groupsChanged (const QStringList&);
		void chatPartStateChanged (const ChatPartState&, const QString&);
	};
};
};
};

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_ENTRYBASE_H
