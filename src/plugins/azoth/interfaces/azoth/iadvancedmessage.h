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

#ifndef PLUGINS_AZOTH_INTERFACES_IADVANCEDMESSAGE_H
#define PLUGINS_AZOTH_INTERFACES_IADVANCEDMESSAGE_H
#include <QtPlugin>

namespace LeechCraft
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

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IAdvancedMessage,
	"org.Deviant.LeechCraft.Azoth.IAdvancedMessage/1.0");

#endif
