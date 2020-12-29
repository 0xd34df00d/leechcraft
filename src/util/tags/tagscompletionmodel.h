/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringListModel>
#include <QStringList>
#include "tagsconfig.h"

namespace LC::Util
{
	/** @brief A QStringListModel providing additional methods for tags.
	 *
	 * This model is basically a QStringListModel with additional methods
	 * for easy working with tags.
	 */
	class UTIL_TAGS_API TagsCompletionModel : public QStringListModel
	{
		Q_OBJECT
	public:
		using QStringListModel::QStringListModel;

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
