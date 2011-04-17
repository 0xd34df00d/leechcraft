/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

namespace LeechCraft
{
namespace Azoth
{
	QString AffToString (IMUCEntry::MUCAffiliation aff)
	{
		switch (aff)
		{
		case IMUCEntry::MUCAInvalid:
			return QObject::tr ("invalid");
		case IMUCEntry::MUCANone:
			return QObject::tr ("none");
		case IMUCEntry::MUCAOutcast:
			return QObject::tr ("banned");
		case IMUCEntry::MUCAMember:
			return QObject::tr ("member");
		case IMUCEntry::MUCAAdmin:
			return QObject::tr ("admin");
		case IMUCEntry::MUCAOwner:
			return QObject::tr ("owner");
		}
		return QString ();
	}
	
	QString RoleToString (IMUCEntry::MUCRole role)
	{
		switch (role)
		{
		case IMUCEntry::MUCRInvalid:
			return QObject::tr ("invalid");
		case IMUCEntry::MUCRNone:
			return QObject::tr ("none");
		case IMUCEntry::MUCRVisitor:
			return QObject::tr ("visitor");
		case IMUCEntry::MUCRParticipant:
			return QObject::tr ("participant");
		case IMUCEntry::MUCRModerator:
			return QObject::tr ("moderator");
		}
		return QString ();
	}
}
}
