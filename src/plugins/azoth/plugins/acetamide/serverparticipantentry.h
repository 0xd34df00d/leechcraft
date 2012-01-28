/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011 Oleg Linkin
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

#ifndef PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H
#define PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H

#include <memory>
#include <QObject>
#include <QStringList>
#include "ircparticipantentry.h"
#include "localtypes.h"

namespace LeechCraft
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

		QObject* GetParentCLEntry () const;

		QString GetEntryID () const;
		QString GetHumanReadableID () const;

		QStringList Groups () const;
		void SetGroups (const QStringList&);

		QObject* CreateMessage (IMessage::MessageType,
				const QString&, const QString&);

		void SetMessageHistory (QObjectList messages);
	private slots:
	};

	typedef std::shared_ptr<ServerParticipantEntry> ServerParticipantEntry_ptr;
}
}
}

#endif // PLUGINS_AZOTH_PLUGINS_ACETAMIDE_SERVERPARTICIPANTENTRY_H
