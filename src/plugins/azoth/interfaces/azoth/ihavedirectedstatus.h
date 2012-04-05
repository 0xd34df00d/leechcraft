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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVEDIRECTEDSTATUS_H
#define PLUGINS_AZOTH_INTERFACES_IHAVEDIRECTEDSTATUS_H
#include <QtPlugin>
#include "iclentry.h"

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for entries for sending directed statuses.
	 *
	 * This interface should be implemented by CL entries that support
	 * directed statuses — that is, statuses sent only to a given entry.
	 *
	 * @sa ICLEntry
	 */
	class IHaveDirectedStatus
	{
	public:
		virtual ~IHaveDirectedStatus () {}

		/** @brief Checks if status can be sent to the given variant.
		 *
		 * This function should return false if directed status cannot
		 * be  sent to variant right now, otherwise it should return
		 * true (even in doubt). A naive implementation may choose to
		 * always return true here.
		 *
		 * @param[in] variant The variant of the entry to check.
		 * @return Whether directed status can be sent to the given
		 * variant.
		 */
		virtual bool CanSendDirectedStatusNow (const QString& variant) = 0;

		/** @brief Sends directed status to the given variant.
		 *
		 * This function should (try to) send the given status to the
		 * given variant of the entry.
		 *
		 * @param[in] status The status to send.
		 * @param[in] variant The variant of the entry where to send the
		 * status.
		 */
		virtual void SendDirectedStatus (const EntryStatus& status, const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveDirectedStatus,
		"org.Deviant.LeechCraft.Azoth.IHaveDirectedStatus/1.0");

#endif
