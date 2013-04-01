/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <util/utilconfig.h>
#include "pagenotification.h"

namespace Ui
{
	class FindNotification;
}

namespace LeechCraft
{
namespace Util
{
	/** @brief A horizontal bar with typical widgets for text search.
	 *
	 * This widget provides typical features for text searching: a text
	 * input field, checkboxes for selecting find mode and buttons for
	 * searching and closing the notification, as well as convenience
	 * slots findNext() and findPrevious().
	 *
	 * The notification will automatically embed into a QVBoxLayout of
	 * its parent widget at the bottom, like in Poshuku or Monocle
	 * plugins.
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
	 */
	class UTIL_API FindNotification : public PageNotification
	{
		Q_OBJECT

		Ui::FindNotification *Ui_;
	public:
		/** Various options controlling the search behavior.
		 */
		enum FindFlag
		{
			/** Search should be performed case sensitively.
			 */
			FindCaseSensitively,

			/** Search should be performed in the reverse direction.
			 */
			FindBackwards,

			/** Search should continue from the beginning when the end is
			 * reached (or from the end if the beginning is reached and
			 * FindBackwards is also set).
			 */
			FindWrapsAround
		};
		Q_DECLARE_FLAGS (FindFlags, FindFlag)

		/** @brief Creates the search widget and embeds into parent layout.
		 *
		 * Embedding is done only if possible — that is, if parent's
		 * layout is QVBoxLayout. Otherwise one should place this widget
		 * where needed himself.
		 */
		FindNotification (QWidget*);
		~FindNotification ();

		/** @brief Sets the text in the find field.
		 *
		 * @param[in] text The text to set in find field.
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

		/** @brief Sets the focus to the search edit field.
		 *
		 * @deprecated This function is deprecated,
		 * <code>QWidget::setFocus()</code> should be used instead.
		 */
		void Focus ();

		/** @brief Returns the current find flags except the direction.
		 *
		 * Please note that the direction flag (FindBackwards) never
		 * appears in the return result.
		 *
		 * @return The find flags corresponding to the user choices.
		 */
		FindFlags GetFlags () const;
	protected:
		/** @brief Called each time the user requests a search.
		 *
		 * Reimplement this function to perform the actual search.
		 *
		 * @param[in] text The text to search for.
		 * @param[in] flags The flags to search with.
		 */
		virtual void handleNext (const QString& text, FindFlags flags) = 0;
	public slots:
		/** @brief Search for the next occurrence of the current search.
		 */
		void findNext ();
		/** @brief Search for the previous occurrence of the current search.
		 */
		void findPrevious ();
	private slots:
		void on_Pattern__textChanged (const QString&);
		void on_FindButton__released ();
		virtual void reject ();
	};
}
}
