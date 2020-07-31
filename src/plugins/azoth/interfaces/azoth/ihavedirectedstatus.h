/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVEDIRECTEDSTATUS_H
#define PLUGINS_AZOTH_INTERFACES_IHAVEDIRECTEDSTATUS_H
#include <QtPlugin>
#include "iclentry.h"

namespace LC
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

Q_DECLARE_INTERFACE (LC::Azoth::IHaveDirectedStatus,
		"org.Deviant.LeechCraft.Azoth.IHaveDirectedStatus/1.0")

#endif
