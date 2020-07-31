/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ICHATSTYLERESOURCESOURCE_H
#define PLUGINS_AZOTH_INTERFACES_ICHATSTYLERESOURCESOURCE_H
#include "iresourceplugin.h"

class QUrl;

namespace LC
{
namespace Azoth
{
	class IAccount;

	/** @brief Defines additional parameters of the message.
	 */
	struct ChatMsgAppendInfo
	{
		/** @brief Whether this message has triggered a highlight.
		 */
		bool IsHighlightMsg_;

		/** @brief Whether this message is appended in an active session.
		 *
		 * The active session is the session that is currently opened.
		 */
		bool IsActiveChat_;

		/** @brief Whether rich text body should be used (if available).
		 */
		bool UseRichTextBody_;

		/** @brief The account corresponding to this message.
		 */
		IAccount *Account_;
	};

	/** @brief Interface for chat style resource loaders and handlers.
	 *
	 * This interface should be implemented by resource sources that are
	 * willing to provide chat styles for Azoth.
	 *
	 * The basic HTML template to be installed into the chat window
	 * whenever a new chat window is created is returned by
	 * GetHTMLTemplate. AppendMessage is used to append a message into
	 * a chat window with an HTML template already set. FrameFocused is
	 * called whenever user focuses on the given chat window.
	 */
	class IChatStyleResourceSource : public IResourceSource
	{
	public:
		virtual ~IChatStyleResourceSource () {}

		/** @brief Returns the base URL for the given style.
		 *
		 * The returned URL would be passed as base URL to the
		 * QWebView::setHtml() method.
		 *
		 * Return a proper value if your theme may contain elements with
		 * relative URIs for them to be loaded properly. Though, if your
		 * theme is known to not have such elements, it's safe to return
		 * a default-constructed QUrl object.
		 *
		 * @param[in] style The style name for which to return the base
		 * URL.
		 * @return The base URL (or default-constructed one).
		 */
		virtual QUrl GetBaseURL (const QString& style) const = 0;

		/** @brief Returns the base HTML template for the given style.
		 *
		 * This function is called whenever a new chat window is being
		 * opened and its chat view is being initialized. The string
		 * returned by this function is then passed to the chat view (to
		 * the QWebView::setHtml() method, to be exact).
		 *
		 * The style parameter indicates the exact style name for which
		 * the template should be returned. The style is the one from
		 * the model returned by IResourceSource::GetOptionsModel().
		 *
		 * The entry parameter defines the entry for which the chat
		 * window is being created. Of course, the entry implements at
		 * least the ICLEntry interface.
		 *
		 * @param[in] style The style name for which to return the
		 * template.
		 * @param[in] variant The style variant for which to return the
		 * template, an element of the list returned by GetVariantsForPack().
		 * @param[in] entry The entry object for which the chat window
		 * is being set up.
		 * @param[in] frame The frame that's being set up.
		 * @return The HTML template.
		 *
		 * @sa GetVariantsForPack()
		 */
		virtual QString GetHTMLTemplate (const QString& style,
				const QString& variant, QObject *entry, QWebFrame *frame) const = 0;

		/** @brief Appends a new message to the chat view.
		 *
		 * This function is called whenever a new message should be
		 * appended to the chat view located in the given frame.
		 *
		 * @param[in] frame The chat view frame.
		 * @param[in] message The message to be appended.
		 * @param[in] info Additional chat message info structure.
		 * @return true on success, false otherwise.
		 */
		virtual bool AppendMessage (QWebFrame *frame, QObject *message,
				const ChatMsgAppendInfo& info) = 0;

		/** @brief Notifies about a frame obtaining user input focus.
		 *
		 * This function is called whenever a given frame receives user
		 * input focus.
		 *
		 * It may be used, for example, to visually separate already
		 * read messages from new ones since the user last focused on
		 * the given chat session.
		 *
		 * @param[in] frame The frame that received focus.
		 */
		virtual void FrameFocused (QWebFrame *frame) = 0;

		/** @brief Returns the list of variants for the \em style pack.
		 *
		 * Values from the returned list will be passed to the
		 * GetHTMLTemplate() function. If the style plugin does not
		 * support style variants, an empty list may be returned.
		 *
		 * @return The list of variants for \em pack, or an empty list
		 * if not supported.
		 *
		 * @sa GetHTMLTemplate()
		 */
		virtual QStringList GetVariantsForPack (const QString& style) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IChatStyleResourceSource,
		"org.Deviant.LeechCraft.Azoth.IChatStyleResourceSource/1.0")

#endif
