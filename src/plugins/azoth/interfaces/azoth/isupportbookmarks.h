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
