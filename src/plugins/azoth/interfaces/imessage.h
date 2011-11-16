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

#ifndef PLUGINS_AZOTH_INTERFACES_IMESSAGE_H
#define PLUGINS_AZOTH_INTERFACES_IMESSAGE_H
#include <QString>
#include <QDateTime>
#include <QtPlugin>

class QObject;

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;

	/** @brief This interface is used to represent a message.
	 *
	 * Refer to the MessageType enum for the list of possible message
	 * types that are covered by this interface.
	 *
	 * The message should not be sent upon creation, only call to Send()
	 * should trigger the sending.
	 *
	 * This interface provides only more or less basic functionality.
	 * Advanced features like delivery receipts and such, are in
	 * IAdvancedMessage.
	 *
	 * Messages implementing this interface are expected to contain only
	 * plain text bodies. If a message may also contain rich text, it
	 * should implement the IRichTextMessage interface.
	 *
	 * @sa IAdvancedMessage, IRichTextMessage.
	 */
	class IMessage
	{
	public:
		virtual ~IMessage () {};

		/** @brief Represents the direction of the message.
		 */
		enum Direction
		{
			/** @brief The message is from the remote party to us.
			 */
			DIn,

			/** @brief The message is from us to the remote party.
			 */
			DOut
		};

		/** @brief Represents possible message types.
		 */
		enum MessageType
		{
			/** @brief Standard one-to-one message.
			 */
			MTChatMessage,

			/** @brief Message in a multiuser conference.
			 *
			 * This kind of message is only for messages that originate
			 * from participants and are human-generated. Thus, status
			 * changes, topic changes and such should have a different
			 * type.
			 */
			MTMUCMessage,

			/** @brief Status changes in a chat.
			 *
			 * This type of message contains information about
			 * participant's status/presence changes.
			 */
			MTStatusMessage,

			/** @brief Various events in a chat.
			 *
			 * Messages of this type are for notifying about, for
			 * example, topic changes, kicks, bans, etc. Generally,
			 * there is no other part in such messages, so the message
			 * of this type can return NULL from OtherPart().
			 */
			MTEventMessage,

			/** @brief Other.
			 */
			MTServiceMessage
		};

		/** @brief This enum is used for more precise classification of
		 * chat types messages.
		 *
		 * The messages of some particular types may have additional
		 * required properties used by the Azoth Core and other plugins
		 * to establish proper context for the events.
		 */
		enum MessageSubType
		{
			/** This message is of subtype that doesn't correspond to
			 * any other subtype of message.
			 */
			MSTOther,

			/** This message notifies about someone being just kicked.
			 */
			MSTKickNotification,

			/** This message notifies about someone being just banned.
			 */
			MSTBanNotification,

			/** @brief Represents status change of a participant in a
			 * chat or MUC room.
			 *
			 * The corresponding MessageType is MTStatusMessage.
			 *
			 * Messages of this type should have the following dynamic
			 * properties:
			 * - Azoth/Nick, with a QString representing nick of the
			 *   participant that has changed its status.
			 * - Azoth/TargetState, with a QString representing the
			 *   target state. The string is better obtained from the
			 *   State enum by the means of IProxyObject::StateToString
			 *   method.
			 * - Azoth/StatusText, with a QString representing the new
			 *   status text of the participant. May be empty.
			 */
			MSTParticipantStatusChange,

			/** @brief Represents permission changes of a participant in
			 * a chat or MUC room.
			 */
			MSTParticipantRoleAffiliationChange,

			/** @brief Notifies about participant joining to a MUC room.
			 */
			MSTParticipantJoin,

			/** @brief Notifies about participant leaving a MUC room.
			 */
			MSTParticipantLeave,

			/** @brief Notifies about participant in a MUC changing the
			 * nick.
			 */
			MSTParticipantNickChange,

			/** @brief The participant has ended the conversation.
			 */
			MSTParticipantEndedConversation,

			/** @brief Notifies about changing subject in a MUC room.
			 */
			MSTRoomSubjectChange
		};

		/** @brief Returns this message as a QObject.
		 */
		virtual QObject* GetObject () = 0;

		/** @brief Sends the message.
		 *
		 * A message should never be sent except as the result of this
		 * method.
		 *
		 * Please note that if the other part is a MUC, it should send
		 * back this message with the "IN" direction set.
		 */
		virtual void Send () = 0;

		/** @brief Returns the direction of this message.
		 *
		 * @return The direction of this message.
		 */
		virtual Direction GetDirection () const = 0;

		/** @brief Returns the type of this message.
		 *
		 * @return The type of this message.
		 */
		virtual MessageType GetMessageType () const = 0;

		/** @brief Returns the subtype of this message.
		 *
		 * The subtype is used for more precise classification of
		 * messages.
		 *
		 * @return The subtype of this message
		 */
		virtual MessageSubType GetMessageSubType () const = 0;

		/** @brief Returns the CL entry from which this message is.
		 *
		 * For normal, single user chats, this should always be equal to
		 * the ICLEntry that was (and will be) used to send the message
		 * back.
		 *
		 * For multiuser chats this should be equal to the contact list
		 * representation of the participant that sent the message.
		 *
		 * The returned object must implement ICLEntry.
		 *
		 * @return The CL entry from which this message originates,
		 * implementing ICLEntry.
		 */
		virtual QObject* OtherPart () const = 0;

		/** @brief Returns the parent CL entry of this message.
		 *
		 * This is the same that OtherPart() for single user chats. For
		 * multiuser chats it should be the contact list entry
		 * representing the MUC room.
		 *
		 * By default this function calls OtherPart() and returns its
		 * result.
		 *
		 * The returned object must implement ICLEntry.
		 *
		 * @return The parent CL entry of this message, implementing
		 * ICLEntry.
		 */
		virtual QObject* ParentCLEntry () const
		{
			return OtherPart ();
		}

		/** @brief The variant of the other part.
		 *
		 * If not applicable, a null string should be returned.
		 *
		 * @return The variant of the other part.
		 */
		virtual QString GetOtherVariant () const = 0;

		/** @brief Returns the body of the message.
		 *
		 * The body is expected to be a plain text string. All '<' and
		 * '&' will be escaped.
		 *
		 * @return The body of the message.
		 */
		virtual QString GetBody () const = 0;

		/** @brief Updates the body of the message.
		 *
		 * The passed string is the plain text contents of the message.
		 *
		 * @param[in] body The new body of the message.
		 */
		virtual void SetBody (const QString& body) = 0;

		/** @brief Returns the timestamp of the message.
		 *
		 * @return The timestamp of the message.
		 */
		virtual QDateTime GetDateTime () const = 0;

		/** @brief Updates the timestamp of the message.
		 *
		 * @param[in] timestamp The new timestamp of the message.
		 */
		virtual void SetDateTime (const QDateTime& timestamp) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMessage,
	"org.Deviant.LeechCraft.Azoth.IMessage/1.0");

#endif
