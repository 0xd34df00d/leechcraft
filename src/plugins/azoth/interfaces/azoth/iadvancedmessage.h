/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IADVANCEDMESSAGE_H
#define PLUGINS_AZOTH_INTERFACES_IADVANCEDMESSAGE_H
#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	/** This interface defines some advanced properties, actions and
	 * signals on messages, like delivery status.
	 *
	 * Objects implementing this interface should, of course, implement
	 * plain IMessage as well.
	 *
	 * @sa IMessage
	 */
	class IAdvancedMessage
	{
	public:
		virtual ~IAdvancedMessage () {}

		/** @brief Queries message delivery status.
		 *
		 * This function returning false doesn't necessarily mean that
		 * the message hasn't been delivered, it may also mean that the
		 * target entry just doesn't support message delivery receipts.
		 *
		 * @return true if the message has surely been delivered, false
		 * if we haven't got delivery receipt yet.
		 */
		virtual bool IsDelivered () const = 0;
	protected:
		/** @brief Notifies that the message has been delivered.
		 *
		 * Please note that this signal may never be emitted at all, for
		 * example, if the target entry doesn't support notifying us
		 * back about message delivery status.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void messageDelivered () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IAdvancedMessage,
	"org.Deviant.LeechCraft.Azoth.IAdvancedMessage/1.0")

#endif
