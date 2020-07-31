/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QDateTime>
#include <QtDebug>
#include <interfaces/azoth/imessage.h>

namespace LC
{
namespace Azoth
{
namespace AzothUtil
{
	namespace detail
	{
		inline IMessage* GetIMessage (IMessage *msg)
		{
			return msg;
		}

		template<typename T>
		IMessage* GetIMessage (T *msgObj)
		{
			return qobject_cast<IMessage*> (msgObj);
		}
	}

	/** @brief Standard function to purge \em messages before the given
	 * date.
	 *
	 * This function is a standard implementation to be used in
	 * LC::Azoth::ICLEntry::PurgeMessages() implementations. It
	 * deletes all the messages in the \em messages list before the given
	 * date \em before.
	 *
	 * The list of \em messages is assumed to be sorted according to the
	 * message timestamp in ascending order.
	 *
	 * If \em before is invalid all the \em messages will be erased and
	 * the list will be cleared.
	 *
	 * @param[inout] messages The list of messages to check.
	 * @param[in] before Messages with timestamp earlier than this
	 * parameter will be erased.
	 * @tparam T The type of the message object, which should be
	 * implementing the IMessage interface.
	 *
	 * @sa IMessage
	 */
	template<typename T>
	void StandardPurgeMessages (QList<T*>& messages, const QDateTime& before)
	{
		if (!before.isValid ())
		{
			qDeleteAll (messages);
			messages.clear ();
			return;
		}

		while (!messages.isEmpty ())
		{
			const auto msg = detail::GetIMessage (messages.at (0));
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< messages.at (0)
						<< "to IMessage; just blindly removing it and hoping for the best";
				messages.removeAt (0);
				continue;
			}
			if (msg->GetDateTime () < before)
				delete messages.takeAt (0);
			else
				break;
		}
	}
}
}
}
