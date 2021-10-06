/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_CHANNELPARTICIPANTENTRY_H
#define LEECHCRAFT_AZOTH_PLUGINS_ACETAMIDE_CHANNELPARTICIPANTENTRY_H

#include <memory>
#include "ircparticipantentry.h"
#include "localtypes.h"

namespace LC
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

		ICLEntry* GetParentCLEntry () const;

		QString GetEntryID () const;
		QString GetHumanReadableID () const;

		void SetGroups (const QStringList&);
		QStringList Groups () const;

		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);

		QList<ChannelRole> Roles () const;
		ChannelRole HighestRole () const;

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
