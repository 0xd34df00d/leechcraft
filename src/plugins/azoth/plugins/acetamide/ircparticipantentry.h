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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARTICIPANTENTRY_H

#include "entrybase.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;

	class IrcParticipantEntry : public EntryBase
	{
		Q_OBJECT
	protected:
		QString Nick_;
		QString UserName_;
		QString HostName_;
		QString RealName_;
		QString ServerID_;
		bool IsPrivateChat_;
	public:
		IrcParticipantEntry (const QString&, IrcAccount* = 0);

		QObject* GetParentAccount () const;

		ICLEntry::Features GetEntryFeatures () const;
		ICLEntry::EntryType GetEntryType () const;

		QString GetEntryName () const;
		void SetEntryName (const QString&);

		virtual QString GetEntryID () const = 0;

		QStringList Variants () const;

		QString GetUserName () const;
		void SetUserName (const QString& user);

		QString GetHostName () const;
		void SetHostName (const QString& host);

		QString GetRealName () const;
		void SetRealName (const QString& realName);
		
		QString GetServerID () const;

		bool IsPrivateChat () const;
		void SetPrivateChat (bool isPrivate);
	private slots:
		void handleClosePrivate ();
	};
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_IRCPARTICIPANTENTRY_H
