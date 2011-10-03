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

#ifndef PLUGINS_AZOTH_INTERFACES_ILASTACTIVITYPROVIDER_H
#define PLUGINS_AZOTH_INTERFACES_ILASTACTIVITYPROVIDER_H
#include <QMetaType>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for plugins providing last activity info.
	 *
	 * This interface should be implemented by plugins (yes, plugin
	 * instance objects) that may provide information about
	 * inactivity timeout.
	 */
	class ILastActivityProvider
	{
	public:
		virtual ~ILastActivityProvider () {}

		/** @brief Number of seconds of inactivity.
		 *
		 * This method returns the number of seconds the user has been
		 * inactive.
		 *
		 * @return Number of seconds of inactivity.
		 */
		virtual int GetInactiveSeconds () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ILastActivityProvider,
		"org.Deviant.LeechCraft.Azoth.ILastActivityProvider/1.0");

#endif
