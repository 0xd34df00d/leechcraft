/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTBOOKMARKS_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTBOOKMARKS_H
#include <QtGlobal>
#include <QVariant>

class QWidget;

namespace LC
{
namespace Azoth
{
	/** @brief Interface for accounts supporting bookmarks.
	 *
	 * This interface should be implemented by accounts that support
	 * bookmarking conferences.
	 *
	 * If this interface is implemented by an entry, the parent account
	 * must also implement IMUCProtocol.
	 *
	 * @sa IMUCProtocol
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

Q_DECLARE_INTERFACE (LC::Azoth::ISupportBookmarks,
		"org.Deviant.LeechCraft.Azoth.ISupportBookmarks/1.0")

#endif
