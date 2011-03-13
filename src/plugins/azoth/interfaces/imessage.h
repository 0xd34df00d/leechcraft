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

	class IMessage
	{
	public:
		virtual ~IMessage () {};

		enum Direction
		{
			DIn,
			DOut
		};

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

		/** This enum is used for more precise classification
		 * of chat types messages.
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
			MSTKickNotification,
			MSTBanNotification,
			
			/** @brief Represents status change of participant in a chat
			 * or MUC room.
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
			MSTParticipantRoleAffiliationChange,
			MSTParticipantJoin,
			MSTParticipantLeave,
			MSTParticipantNickChange,
			MSTRoomSubjectChange
		};

		virtual QObject* GetObject () = 0;

		/** Please note that if the other part is a MUC, it should send
		 * back this message with the "IN" direction set.
		 */
		virtual void Send () = 0;
		virtual Direction GetDirection () const = 0;
		virtual MessageType GetMessageType () const = 0;
		virtual MessageSubType GetMessageSubType () const = 0;

		/** The contact list entry from which this message originates.
		 *
		 * For normal, single user chats, this should always equal to
		 * the ICLEntry that was (and will be) used to send the message
		 * back.
		 *
		 * For multiuser chats this should be equal to the contact list
		 * representation of the participant that sent the message.
		 */
		virtual QObject* OtherPart () const = 0;

		/** This is the same that OtherPart() for single user chats. For
		 * multiuser chats it should be the contact list entry
		 * representing the MUC room.
		 *
		 * By default this function calls OtherPart() and returns its
		 * result.
		 */
		virtual QObject* ParentCLEntry () const
		{
			return OtherPart ();
		}

		virtual QString GetOtherVariant () const = 0;
		virtual QString GetBody () const = 0;
		virtual void SetBody (const QString&) = 0;
		virtual QDateTime GetDateTime () const = 0;
		virtual void SetDateTime (const QDateTime&) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IMessage,
	"org.Deviant.LeechCraft.Azoth.IMessage/1.0");

#endif
