/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_IRICHTEXTMESSAGE_H
#define PLUGINS_AZOTH_INTERFACES_IRICHTEXTMESSAGE_H
#include <QtPlugin>

namespace LC
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

Q_DECLARE_INTERFACE (LC::Azoth::IRichTextMessage,
	"org.Deviant.LeechCraft.Azoth.IRichTextMessage/1.0")

#endif
