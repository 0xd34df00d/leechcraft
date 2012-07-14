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

#ifndef LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_CHANNELPARTICIPANTENTRY_H
#define LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_CHANNELPARTICIPANTENTRY_H

#include <memory>
#include "ircparticipantentry.h"
#include "localtypes.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Acetamide
{
	class IrcAccount;
	class ChannelHandler;

	class ChannelParticipantEntry : public IrcParticipantEntry
	{
		Q_OBJECT

		ChannelHandler *ICH_;
		QList<ChannelRole> Roles_;
	public:
		ChannelParticipantEntry (const QString&,
				ChannelHandler*, IrcAccount* = 0);

		QObject* GetParentCLEntry () const;

		QString GetEntryID () const;
		QString GetHumanReadableID () const;

		void SetGroups (const QStringList&);
		QStringList Groups () const;

		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		QList<ChannelRole> Roles () const;
		ChannelRole HighestRole ();

		void SetRole (const ChannelRole& role);
		void SetRoles (const QList<ChannelRole>& roles);
		void RemoveRole (const ChannelRole& role);
	private slots:
		void handleWhoIs ();
		void handleWhoWas ();
		void handleWho ();
		void handleCTCPAction (QAction *action);
	};

	typedef std::shared_ptr<ChannelParticipantEntry> ChannelParticipantEntry_ptr;
}
}
}

#endif // LEECHCRAFT_AZOTH_ACETAMIDE_CHANNELPARTICIPANTENTRY_H
