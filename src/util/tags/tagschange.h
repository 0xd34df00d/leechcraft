/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include "tagsconfig.h"

namespace LC::Util
{
	/** @brief A change of a tags list, described by its old and new versions.
	 *
	 * The versions are compared as sets: neither the order of the tags
	 * nor their repetitions matter.
	 */
	struct UTIL_TAGS_API TagsChange
	{
		QStringList Before_;
		QStringList After_;

		/** @brief Returns the tags of After_ missing from Before_.
		 *
		 * @return The added tags, each one once, in the order of After_.
		 */
		QStringList Added () const;

		/** @brief Returns the tags of Before_ missing from After_.
		 *
		 * @return The removed tags, each one once, in the order of Before_.
		 */
		QStringList Removed () const;

		/** @brief Returns whether this change neither adds nor removes tags.
		 */
		bool IsEmpty () const;

		/** @brief Applies this change to another tags list.
		 *
		 * The removed tags are removed from \em tags, and the added tags
		 * are appended to it unless it already has them. The rest of
		 * \em tags keeps its order.
		 *
		 * @param[in] tags The tags to change.
		 * @return The changed \em tags.
		 */
		QStringList ApplyTo (QStringList tags) const;
	};
}
