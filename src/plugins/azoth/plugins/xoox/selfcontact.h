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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_SELFCONTACT_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_SELFCONTACT_H
#include "entrybase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class SelfContact : public EntryBase
	{
		Q_OBJECT

		QString FullJID_;
		QString BareJID_;
		QString Resource_;

		QMap<int, QString> Prio2Status_;
	public:
		SelfContact (const QString&, GlooxAccount*);

		QObject* GetParentAccount () const;
		Features GetEntryFeatures () const;
		EntryType GetEntryType () const;
		QString GetEntryName () const;
		void SetEntryName (const QString&);
		QString GetEntryID () const;
		QString GetHumanReadableID () const;
		QStringList Groups () const;
		void SetGroups (const QStringList&);
		QStringList Variants () const;
		EntryStatus GetStatus (const QString&) const;
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);
		QList<QAction*> GetActions () const;

		void UpdatePriority (const QString&, int);
		void RemoveVariant (const QString&);
		QString GetJID () const;
		void UpdateJID (const QString&);
	private slots:
		void handleSelfVCardUpdated ();
	};
}
}
}

#endif
