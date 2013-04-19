/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTBOOKMARKS_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTBOOKMARKS_H
#include <QtGlobal>
#include <QVariant>

class QWidget;

namespace LeechCraft
{
namespace Azoth
{
	/** @brief Interface for accounts supporting bookmarks.
	 *
	 * This interface should be implemented by accounts that support
	 * bookmarking conferences.
	 */
	class ISupportBookmarks
	{
	public:
		virtual ~ISupportBookmarks () {}

		/** @brief Returns the editor widget for the bookmarks of this
		 * protocol.
		 *
		 * The returned widget must implement the
		 * IMUCBookmarkEditorWidget interface.
		 *
		 * This function should create a new widget each time it is
		 * called, since the ownership is transferred to the caller and
		 * the widget will be deleted by the caller when appropriate.
		 *
		 * @sa IMUCBookmarkEditorWidget
		 */
		virtual QWidget* GetMUCBookmarkEditorWidget () = 0;

		/** @brief Returns the list of bookmarked MUCs, if any.
		 *
		 * The returned list is a list of QVariantMaps which should have
		 * the same format as the ones returned from the
		 * GetIdentifyingData(). Please refer to documentation for
		 * GetIdentifyingData() for more information about maps'
		 * contents.
		 *
		 * @return List of bookmarks parameters.
		 *
		 * @sa GetIdentifyingData()
		 */
		virtual QVariantList GetBookmarkedMUCs () const = 0;

		/** @brief Sets the bookmarked MUCs for the given account.
		 *
		 * The passed list is typically based on the result of the
		 * corresponding IMUCBookmarkEditWidget::GetIdentifyingData()
		 * return values.
		 *
		 * The bookmarksChanged() signal should be emitted soon after
		 * this call.
		 *
		 * @param[in] bookmarks The list of variant maps with bookmarks.
		 *
		 * @sa bookmarksChanged()
		 */
		virtual void SetBookmarkedMUCs (const QVariantList& bookmarks) = 0;

		/** @brief Notifies that bookmarks have been changed.
		 *
		 * This signal should be emitted whenever the list of the
		 * bookmarks changes. Particularly, it should be emitted after
		 * SetBookmarkedMUCs() is called, for example, when server
		 * acknowledges bookmarks are saved successfully if they are
		 * stored remotely, or immediately if they are stored locally.
		 *
		 * @sa SetBookmarkedMUCs()
		 */
		virtual void bookmarksChanged () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportBookmarks,
		"org.Deviant.LeechCraft.Azoth.ISupportBookmarks/1.0");

#endif
