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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_RIEXMANAGER_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_RIEXMANAGER_H
#include <QXmppClientExtension.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class EntryBase;

	class RIEXManager : public QXmppClientExtension
	{
		Q_OBJECT
	public:
		class Item
		{
		public:
			enum Action
			{
				AAdd,
				ADelete,
				AModify
			};
		private:
			Action Action_;

			QString JID_;
			QString Name_;
			QStringList Groups_;
		public:
			Item ();
			Item (Action action, QString jid, QString name, QStringList groups);

			Action GetAction () const;
			void SetAction (Action);

			QString GetJID () const;
			void SetJID (QString);

			QString GetName () const;
			void SetName (QString);

			QStringList GetGroups () const;
			void SetGroups (QStringList);
		};

		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);

		void SuggestItems (EntryBase *to, QList<Item> items,
				QString message = QString ());
	signals:
		void gotItems (QString from, QList<RIEXManager::Item> items, bool messagePending);
	};
}
}
}

#endif
