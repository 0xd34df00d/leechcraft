/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

namespace LC
{
namespace Azoth
{
	/** @brief Describes contact mood information.
	 *
	 * It is modeled after XMPP XEP-0107, so please refer to
	 * http://xmpp.org/extensions/xep-0107.html for more information.
	 */
	struct MoodInfo
	{
		/** @brief Mood name as per XEP-0107.
		 *
		 * If this field is empty, the entry is considered to have stopped
		 * publishing mood information.
		 */
		QString Mood_;

		/** @brief Optional contact-set text accompanying the mood.
		 */
		QString Text_;
	};

	/** @brief Checks whether the mood info structures are equal.
	 *
	 * Returns true if \em i1 is equal to \em i2, containing the same
	 * values for all the fields, otherwise returns false.
	 *
	 * @param[in] i1 The first mood info structure.
	 * @param[in] i2 The second mood info structure.
	 * @return Whether \em i1 and \em i2 are equal.
	 */
	inline bool operator== (const MoodInfo& i1, const MoodInfo& i2)
	{
		return i1.Mood_ == i2.Mood_ &&
			   i1.Text_ == i2.Text_;
	}

	/** @brief Checks whether the mood info structures are not equal.
	 *
	 * Returns true if \em i1 is not equal to \em i2, that is, if at
	 * least one field of \em i1 is not equal to the corresponding one of
	 * \em i2. Otherwise returns false.
	 *
	 * @param[in] i1 The first mood info structure.
	 * @param[in] i2 The second mood info structure.
	 * @return Whether \em i1 and \em i2 are not equal.
	 */
	inline bool operator!= (const MoodInfo& i1, const MoodInfo& i2)
	{
		return !(i1 == i2);
	}
}
}
