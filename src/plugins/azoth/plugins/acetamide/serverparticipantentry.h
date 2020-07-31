/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H

#include <memory>
#include <QObject>
#include <QStringList>
#include "ircparticipantentry.h"
#include "localtypes.h"

namespace LC
{
namespace Azoth
{
namespace Acetamide
{

	class IrcAccount;
	class IrcServerHandler;

	class ServerParticipantEntry : public IrcParticipantEntry
	{
		Q_OBJECT

		IrcServerHandler *ISH_;
	public:
		ServerParticipantEntry (const QString&,
				IrcServerHandler*, IrcAccount*);

		ICLEntry* GetParentCLEntry () const;

		QString GetEntryID () const;
		QString GetHumanReadableID () const;

		QStringList Groups () const;
		void SetGroups (const QStringList&);

		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&);

		void SetMessageHistory (QObjectList messages);
	};

	typedef std::shared_ptr<ServerParticipantEntry> ServerParticipantEntry_ptr;
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H
