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

#ifndef PLUGINS_AZOTH_INTERFACES_AZOTHCOMMON_H
#define PLUGINS_AZOTH_INTERFACES_AZOTHCOMMON_H
#include <QMetaType>

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{








	enum State
	{
		SOffline,
		SOnline,
		SAway,
		SXA,
		SDND,
		SChat,
		SInvisible,
		SProbe,
		SError,
		SInvalid
	};

	inline bool IsLess (State s1, State s2)
	{
		static int order [] = { 7, 3, 4, 5, 6, 1, 2, 8, 9, 10 };
		return order [s1] < order [s2];
	}

	/** Represents possible state of authorizations between two
	 * entities: our user and a remote contact.
	 *
	 * Modelled after RFC 3921, Section 9.
	 */
	enum AuthStatus
	{
		/** Contact and user are not subscribed to each other, and
		 * neither has requested a subscription from the other.
		 */
		ASNone,

		/** Contact is subscribed to user (one-way).
		 */
		ASFrom,

		/** User is subscribed to contact (one-way).
		 */
		ASTo,

		/** User and contact are subscribed to each other (two-way).
		 */
		ASBoth
	};
}
}
}
}

Q_DECLARE_METATYPE (LeechCraft::Plugins::Azoth::Plugins::State);

#endif
