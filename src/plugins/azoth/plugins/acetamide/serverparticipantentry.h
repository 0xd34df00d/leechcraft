/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QCoreApplication>
#include <QStringList>
#include "ircparticipantentry.h"

namespace LC::Azoth::Acetamide
{
	class IrcAccount;
	class IrcServerHandler;

	class ServerParticipantEntry : public IrcParticipantEntry
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Azoth::Acetamide::ServerParticipantEntry)

		IrcServerHandler * const ISH_;
	public:
		ServerParticipantEntry (QString, IrcServerHandler*, IrcAccount*);

		ICLEntry* GetParentCLEntry () const override;

		QString GetEntryID () const override;
		QString GetHumanReadableID () const override;

		QStringList Groups () const override;
		void SetGroups (const QStringList&) override;

		IMessage* CreateMessage (IMessage::Type, const QString&, const QString&) override;
	};

	using ServerParticipantEntry_ptr = std::shared_ptr<ServerParticipantEntry>;
}
