/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_UNAUTHCLENTRY_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_UNAUTHCLENTRY_H
#include "entrybase.h"
#include <QStringList>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;

	/** Represents an entry in the contact list that has requested the
	 * authorization but hasn't been granted nor denied yet.
	 */
	class UnauthCLEntry : public EntryBase
	{
		Q_OBJECT

		QString JID_;
		GlooxAccount *Account_;
		QStringList Groups_;
	public:
		UnauthCLEntry (const QString&, const QString&, GlooxAccount*);

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
		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		QString GetJID () const;
	};
}
}
}

#endif
