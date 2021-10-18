/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QCoreApplication>
#include "ircparticipantentry.h"
#include "localtypes.h"

namespace LC::Azoth::Acetamide
{
	class IrcAccount;
	class ChannelHandler;

	class ChannelParticipantEntry : public IrcParticipantEntry
	{
		Q_OBJECT

		ChannelHandler * const ICH_;
		QList<ChannelRole> Roles_;
	public:
		ChannelParticipantEntry (const QString&, ChannelHandler*, IrcAccount* = nullptr);

		ICLEntry* GetParentCLEntry () const override;

		QString GetEntryID () const override;
		QString GetHumanReadableID () const override;

		void SetGroups (const QStringList&) override;
		QStringList Groups () const override;

		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&) override;

		QList<ChannelRole> Roles () const;
		ChannelRole HighestRole () const;

		void SetRole (ChannelRole role);
		void SetRoles (const QList<ChannelRole>& roles);
		void RemoveRole (ChannelRole role);
	};

	using ChannelParticipantEntry_ptr = std::shared_ptr<ChannelParticipantEntry>;
}
