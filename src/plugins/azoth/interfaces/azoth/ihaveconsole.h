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

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#define PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#include <QMetaType>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for accounts that support protocol consoles.
	 * 
	 * An example of a protocol console may be an XML console for XMPP
	 * protocol or just raw text console for IRC.
	 * 
	 * The account supporting console notifies about new packets (both
	 * incoming and outgoing) by the gotConsolePacket() signal.
	 * 
	 * Azoth core and other plugins may toggle the status of the console
	 * by the means of SetConsoleEnabled() function. By default, the
	 * console for each account should be disabled.
	 * 
	 * @sa IAccount
	 */
	class IHaveConsole
	{
	public:
		virtual ~IHaveConsole () {}
		
		/** @brief Defines the format of the packets in this protocol.
		 */
		enum PacketFormat
		{
			/** @brief XML packets (like in XMPP).
			 * 
			 * The packets would be represented as formatted XML text.
			 */
			PFXML,
			
			/** @brief Plain text packets (like in IRC).
			 * 
			 * The packets would be represented as unformatted plain
			 * text.
			 */
			PFPlainText,
			
			/** @brief Binary packets (like in Oscar).
			 * 
			 * The packets would be converted to Base64 or Hex-encoding.
			 */
			PFBinary
		};
		
		/** @brief Defines the direction of a packet.
		 */
		enum PacketDirection
		{
			/** @brief Incoming packet.
			 */
			PDIn,
			
			/** @brief Outgoing packet.
			 */
			PDOut
		};

		/** @brief Returns the packet format used in this account.
		 * 
		 * @return The packet format.
		 */
		virtual PacketFormat GetPacketFormat () const = 0;
		
		/** @brief Enables or disables the console.
		 * 
		 * This function toggles the status of the console for the
		 * corresponding account.
		 * 
		 * If the console is enabled, only this account's packets should
		 * be emitted by the gotConsolePacket() signal. If the console
		 * is disabled, gotConsolePacket() signal shouldn't be emitted
		 * at all.
		 * 
		 * By default, console for each account should be in disabled
		 * state, unless explicitly enabled by calling this function.
		 * 
		 * @param[in] enabled Whether the console should be enabled.
		 */
		virtual void SetConsoleEnabled (bool enabled) = 0;
		
		/** @brief Notifies about new packet.
		 * 
		 * This signal is used by the console-supporting account to
		 * notify about new packets, both incoming and outgoing.
		 * 
		 * This signal should be emitted if and only if the console has
		 * been explicitly enabled for this account by calling the
		 * SetConsoleEnabled() function.
		 * 
		 * @note This function is expected to be a signal.
		 * 
		 * @param[out] packet The packet data.
		 * @param[out] direction The direction of the packet, member of
		 * the PacketDirection enum.
		 */
		virtual void gotConsolePacket (const QByteArray& packet, int direction) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IHaveConsole,
		"org.Deviant.LeechCraft.Azoth.IHaveConsole/1.0");

#endif
