/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <interfaces/core/icoreproxy.h>
#include "guiconfig.h"
#include "pagenotification.h"

class QShortcut;

namespace Ui
{
	class FindNotification;
}

namespace LC::Util
{
	/** @brief A horizontal bar with typical widgets for text search.
	 *
	 * This widget provides typical features for text searching: a text
	 * input field, checkboxes for selecting find mode and buttons for
	 * searching and closing the notification, as well as convenience
	 * slots findNext() and findPrevious().
	 *
	 * The widget will automatically be embedded into the layout of
	 * the parent widget of \em near after the \em near widget (which is
	 * passed to the constructor).
	 *
	 * This class is typically used as following:
	 * -# It's subclassed, and an implementation of handleNext() function
	 *    is provided, which deals with the search process. For example,
	 *    a WebKit-based browser calls <code>QWebPage::findText()</code>.
	 *    The implementation may also call SetSuccessful() to indicate
	 *    whether anything has been found.
	 * -# An object of the subclass is created as a child of some page
	 *    containing searchable text, like a web page or a text document.
	 * -# It's hidden after that to not disturb the user until he
	 *    explicitly wishes to search for text.
	 * -# A QAction is created to trigger showing this notification, and
	 *    its <code>triggered()</code> signal is connected to this class'
	 *    show() and setFocus() slots (latter is needed so that user
	 *    can start typing his search query immediately).
	 * -# Optionally a couple of QShortCuts or QActions can be created
	 *    and connected to findNext() and findPrevious() slots to support
	 *    shortcuts for the corresponding actions.
	 *
	 * The FindNotificationWk class provides some utilities to aid
	 * integrating this class with a QWebPage.
	 *
	 * @sa FindNotificationWk
	 *
	 * @ingroup GuiUtil
	 */
	class UTIL_GUI_API FindNotification : public PageNotification
	{
		Q_OBJECT

		std::unique_ptr<Ui::FindNotification> Ui_;
		QShortcut * const EscShortcut_;
	public:
		/** Various options controlling the search behavior.
		 */
		enum FindFlag
		{
			/** Default search flags: case-insensitive forward search.
			 */
			FindNoFlags = 0x0,

			/** Search should be performed case sensitively.
			 */
			FindCaseSensitively = 0x1,

			/** Search should be performed in the reverse direction.
			 */
			FindBackwards = 0x2,

			/** Search should continue from the beginning when the end is
			 * reached (or from the end if the beginning is reached and
			 * FindBackwards is also set).
			 */
			FindWrapsAround = 0x4
		};
		Q_DECLARE_FLAGS (FindFlags, FindFlag)

		/** @brief Creates the search widget in parent layout of \em near.
		 *
		 * Embedding is done only if possible — that is, if parent's
		 * layout is QVBoxLayout. Otherwise one should place this widget
		 * where needed himself.
		 *
		 * @param[in] proxy The core proxy to be used by this find
		 * notification.
		 * @param[in] near The widget near which to embed.
		 */
		FindNotification (const ICoreProxy_ptr& proxy, QWidget *near);
		~FindNotification () override;

		/** @brief Sets whether Esc closes the widget.
		 *
		 * @param[in] close Whether pressing Esc button closes the widget.
		 */
		void SetEscCloses (bool close);

		/** @brief Sets the text in the find field.
		 *
		 * This does not trigger the search. To perform the search, call
		 * findNext() after calling this method.
		 *
		 * @param[in] text The text to set in find field.
		 *
		 * @sa findNext()
		 */
		void SetText (const QString& text);

		/** @brief Returns the currently entered text in the find field.
		 *
		 * @return Currently entered text in the find field.
		 */
		QString GetText () const;

		/** @brief Updates the widget to show whether the search has been
		 * successful.
		 *
		 * @param[in] successful Whether the search has been successful.
		 */
		void SetSuccessful (bool successful);

		/** @brief Returns the current find flags except the direction.
		 *
		 * Please note that the direction flag (FindBackwards) never
		 * appears in the return result.
		 *
		 * @return The find flags corresponding to the user choices.
		 */
		FindFlags GetFlags () const;

		/** @brief Search for the next occurrence of the search text.
		 *
		 * If the text has been modified since the previous call to
		 * either findPrevious() or findNext() either programmatically
		 * (via SetText()) or by the user, the search will be restarted.
		 *
		 * @sa findPrevious()
		 */
		void FindNext ();

		/** @brief Search for the previous occurrence of the search text.
		 *
		 * If the text has been modified since the previous call to
		 * either findPrevious() or findNext() either programmatically
		 * (via SetText()) or by the user, the search will be restarted.
		 *
		 * @sa findNext()
		 */
		void FindPrevious ();

		/** @brief Clears the text in the find field.
		 *
		 * This is equivalent to <code>SetText ({})</code>.
		 */
		void Clear ();
	protected:
		/** @brief Called each time the user requests a search.
		 *
		 * Reimplement this function to perform the actual search.
		 *
		 * @param[in] text The text to search for.
		 * @param[in] flags The flags to search with.
		 */
		virtual void HandleNext (const QString& text, FindFlags flags) = 0;

		virtual void Reject ();
	};
}
