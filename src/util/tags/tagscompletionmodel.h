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

#include <QStringListModel>
#include <QStringList>
#include <util/utilconfig.h>

namespace LeechCraft
{
namespace Util
{
	/** @brief A QStringListModel providing additional methods for tags.
	 *
	 * This model is basically a QStringListModel with additional methods
	 * for easy working with tags.
	 */
	class UTIL_API TagsCompletionModel : public QStringListModel
	{
		Q_OBJECT
	public:
		/** @brief Creates the model with the given parent.
		 *
		 * @param[in] parent The parent object of this model.
		 */
		TagsCompletionModel (QObject *parent = 0);

		/** @brief Adds new tags to the list of tags.
		 *
		 * This method adds the \em newTags to the already existing list
		 * of tags avoiding duplicates and emits the tagsUpdated()
		 * signal.
		 *
		 * @param[in] newTags The new tags to append to this model.
		 */
		void UpdateTags (const QStringList& newTags);
	signals:
		/** @brief Emitted when tags are updated via UpdateTags().
		 *
		 * @param[in] allTags All tags in this model.
		 */
		void tagsUpdated (const QStringList& allTags);
	};
}
}
