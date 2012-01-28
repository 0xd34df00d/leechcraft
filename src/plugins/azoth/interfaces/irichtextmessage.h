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

#ifndef PLUGINS_AZOTH_INTERFACES_IRICHTEXTMESSAGE_H
#define PLUGINS_AZOTH_INTERFACES_IRICHTEXTMESSAGE_H
#include <QtPlugin>

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for messages supporting rich text contents.
	 * 
	 * If a message supports rich text contents (like HTML formatting or
	 * inline images) and wants to advertise this, it should also
	 * implement this interface in addition to the plain IMessage.
	 * 
	 * The contents returned by GetRichBody() should only supplement the
	 * plain text string returned by IMessage::GetBody(). That is, the
	 * string returned IMessage::GetBody() should have basically the
	 * same meaning as the one from GetRichBody().
	 * 
	 * @sa IMessage
	 */
	class IRichTextMessage
	{
	public:
		virtual ~IRichTextMessage () {}

		/** @brief Returns the rich text contents.
		 * 
		 * The returned string is expected to be an HTML string. It
		 * should be possible to append the string contents as the child
		 * of HTML's body element.
		 * 
		 * The contents of the returned string should have the same
		 * semantic meaning to the user as the ones of the string
		 * returned by the IMessage::GetBody(). Ideally, the latter
		 * should be equal to the contents of this one stripped of all
		 * the HTML formatting.
		 * 
		 * The string contents aren't escaped by the core. All the
		 * required escaping should be done by the interface
		 * implementation itself.
		 * 
		 * @return The HTML-formatted rich text string.
		 * 
		 * @sa SetRichBody(), IMessage::GetBody()
		 */
		virtual QString GetRichBody () const = 0;
		
		/** @brief Sets the rich text contents.
		 * 
		 * This function is called to set the HTML-formatted rich
		 * contents of the message. The passed string satisfies all the
		 * requirements mentioned in the GetRichBody() documentation.
		 * 
		 * This function should NOT update the usual plain-text body of
		 * the message, it would be set separately.
		 * 
		 * @param[in] text The HTML-formatted contents of the message.
		 * 
		 * @sa GetRichBody(), IMessage::SetBody()
		 */
		virtual void SetRichBody (const QString& text) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::IRichTextMessage,
	"org.Deviant.LeechCraft.Azoth.IRichTextMessage/1.0");

#endif
