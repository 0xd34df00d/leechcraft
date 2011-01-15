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

#ifndef PLUGINS_AZOTH_INTERFACES_IAUTHABLE_H
#define PLUGINS_AZOTH_INTERFACES_IAUTHABLE_H
#include "azothcommon.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
	/** @brief Represents an entry that supports authorizations.
	 */
	class IAuthable
	{
	public:
		virtual ~IAuthable () {}

		/** @brief Returns the AuthStatus between our user and this
		 * remote.
		 *
		 * @return Authorization status of this entry.
		 */
		virtual AuthStatus GetAuthStatus () const = 0;
	};
}
}
}
}

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IAuthable,
		"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IAuthable/1.0");

#endif
