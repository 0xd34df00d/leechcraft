/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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
	namespace Plugins
	{
		namespace Azoth
		{
			namespace Plugins
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
						MTChatMessage,
						MTMUCMessage,
						MTService
					};

					virtual QObject* GetObject () = 0;

					/** Please note that if the other part is a MUC, it
					 * should send back this message with the "IN"
					 * direction set.
					 */
					virtual void Send () = 0;
					virtual Direction GetDirection () const = 0;
					virtual MessageType GetMessageType () const = 0;

					/** The contact list entry from which this message
					 * originates.
					 *
					 * For normal, single user chats, this should always
					 * equal to the ICLEntry that was (and will be) used
					 * to send the message back.
					 *
					 * For multiuser chats this should be equal to the
					 * contact list representation of the participant
					 * that sent the message.
					 */
					virtual ICLEntry* OtherPart () const = 0;
					virtual QString GetOtherVariant () const = 0;
					virtual QString GetBody () const = 0;
					virtual void SetBody (const QString&) = 0;
					virtual QDateTime GetDateTime () const = 0;
					virtual void SetDateTime (const QDateTime&) = 0;
				};
			}
		}
	}
}

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Azoth::Plugins::IMessage,
	"org.Deviant.LeechCraft.Plugins.Azoth.Plugins.IMessage/1.0");

#endif
