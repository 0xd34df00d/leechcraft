/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "zheetutil.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Zheet
{
namespace ZheetUtil
{
	std::string ToStd (const QString& str)
	{
		return str.toUtf8 ().constData ();
	}

	QString FromStd (const std::string& str)
	{
		return QString::fromUtf8 (str.c_str ());
	}

	MSN::BuddyStatus ToMSNState (State st)
	{
		switch (st)
		{
		case SOnline:
		case SChat:
			return MSN::STATUS_AVAILABLE;
		case SAway:
			return MSN::STATUS_AWAY;
		case SXA:
			return MSN::STATUS_IDLE;
		case SDND:
			return MSN::STATUS_BUSY;
		case SInvisible:
		default:
			return MSN::STATUS_INVISIBLE;
		}
	}

	State FromMSNState (MSN::BuddyStatus status)
	{
		switch (status)
		{
		case MSN::STATUS_AVAILABLE:
			return SOnline;
		case MSN::STATUS_BUSY:
		case MSN::STATUS_ONTHEPHONE:
			return SDND;
		case MSN::STATUS_IDLE:
			return SXA;
		case MSN::STATUS_AWAY:
		case MSN::STATUS_BERIGHTBACK:
		case MSN::STATUS_OUTTOLUNCH:
			return SAway;
		case MSN::STATUS_INVISIBLE:
			return SInvisible;
		default:
			return SOffline;
		}
	}
}
}
}
}
