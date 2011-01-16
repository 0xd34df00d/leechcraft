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

#include "util.h"
#include <QObject>
#include <QHash>

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
namespace Util
{
	QString RoleToString (const gloox::MUCRoomRole& role)
	{
		switch (role)
		{
		case gloox::RoleInvalid:
			return QObject::tr ("invalid role");
		case gloox::RoleNone:
			return QObject::tr ("guest");
		case gloox::RoleVisitor:
			return QObject::tr ("visitor");
		case gloox::RoleParticipant:
			return QObject::tr ("participant");
		case gloox::RoleModerator:
			return QObject::tr ("moderator");
		}
	}

	QString AffiliationToString (const gloox::MUCRoomAffiliation& aff)
	{
		switch (aff)
		{
		case gloox::AffiliationInvalid:
			return QObject::tr ("invalid affiliation");
		case gloox::AffiliationNone:
			return QObject::tr ("guest");
		case gloox::AffiliationOutcast:
			return QObject::tr ("banned");
		case gloox::AffiliationMember:
			return QObject::tr ("member");
		case gloox::AffiliationAdmin:
			return QObject::tr ("administrator");
		case gloox::AffiliationOwner:
			return QObject::tr ("owner");
		}
	}

	namespace
	{
		struct Node2ClientID
		{
			QHash<QString, QString> Node2ClientID_;

			Node2ClientID ()
			{
				Node2ClientID_ ["http://gajim.org"] = "gajim";
				Node2ClientID_ ["http://leechcraft.org/azoth"] = "leechcraft-azoth";
				Node2ClientID_ ["http://pidgin.im/"] = "pidgin";
				Node2ClientID_ ["http://psi-im.org/caps"] = "psi";
				Node2ClientID_ ["http://psi-dev.googlecode.com/caps"] = "psiplus";
				Node2ClientID_ ["http://pyicqt.googlecode.com//protocol/caps"] = "pyicq-t";
			}
		};
	}

	QString GetClientIDName (const QString& node)
	{
		static Node2ClientID n2ci;
		return n2ci.Node2ClientID_.value (node);
	}

	namespace
	{
		struct Node2ClientHR
		{
			QHash<QString, QString> Node2ClientHR_;

			Node2ClientHR ()
			{
				Node2ClientHR_ ["http://gajim.org"] = "Gajim";
				Node2ClientHR_ ["http://leechcraft.org/azoth"] = "LeechCraft Azoth";
				Node2ClientHR_ ["http://pidgin.im/"] = "Pidgin IM";
				Node2ClientHR_ ["http://psi-im.org/caps"] = "Psi";
				Node2ClientHR_ ["http://psi-dev.googlecode.com/caps"] = "Psi+";
				Node2ClientHR_ ["http://pyicqt.googlecode.com//protocol/caps"] = "PyICQ-t";
			}
		};
	}

	QString GetClientHRName (const QString& node)
	{
		static Node2ClientHR n2ch;
		return n2ch.Node2ClientHR_.value (node);
	}
}
}
}
}
}
}
