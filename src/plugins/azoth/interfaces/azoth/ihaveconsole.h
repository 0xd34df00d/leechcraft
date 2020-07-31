/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#define PLUGINS_AZOTH_INTERFACES_IHAVECONSOLE_H
#include <QMetaType>

namespace LC
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
		enum class PacketFormat
		{
			/** @brief XML packets (like in XMPP).
			 *
			 * The packets would be represented as formatted XML text.
			 */
			XML,

			/** @brief Plain text packets (like in IRC).
			 *
			 * The packets would be represented as unformatted plain
			 * text.
			 */
			PlainText,

			/** @brief Binary packets (like in Oscar).
			 *
			 * The packets would be converted to Base64 or Hex-encoding.
			 */
			Binary
		};

		/** @brief Defines the direction of a packet.
		 */
		enum class PacketDirection
		{
			/** @brief Incoming packet.
			 */
			In,

			/** @brief Outgoing packet.
			 */
			Out
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
	protected:
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
		 * @param[out] hrEntryId The human-readable ID of the related
		 * entry, or null string if not applicable.
		 */
		virtual void gotConsolePacket (const QByteArray& packet,
				PacketDirection direction, const QString& hrEntryId) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveConsole,
		"org.Deviant.LeechCraft.Azoth.IHaveConsole/1.0")

#endif
