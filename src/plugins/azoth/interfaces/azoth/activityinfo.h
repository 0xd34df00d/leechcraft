/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMetaType>

namespace LC
{
namespace Azoth
{
	/** @brief Describes contact activity information.
	 *
	 * It is modeled after XMPP XEP-0108, so please refer to
	 * http://xmpp.org/extensions/xep-0108.html for more information.
	 */
	struct ActivityInfo
	{
		/** @brief General activity name as per XEP-0108.
		 *
		 * If this field is empty, the entry is considered to have stopped
		 * publishing activity information.
		 */
		QString General_;

		/** @brief Specific activity name within the General_ one.
		 */
		QString Specific_;

		/** @brief Optional contact-set text accompanying the activity.
		 */
		QString Text_;
	};

	/** @brief Checks whether the activity info structures are equal.
	 *
	 * Returns true if \em i1 is equal to \em i2, containing the same
	 * values for all the fields, otherwise returns false.
	 *
	 * @param[in] i1 The first activity info structure.
	 * @param[in] i2 The second activity info structure.
	 * @return Whether \em i1 and \em i2 are equal.
	 */
	inline bool operator== (const ActivityInfo& i1, const ActivityInfo& i2)
	{
		return i1.General_ == i2.General_ &&
			   i1.Specific_ == i2.Specific_ &&
			   i1.Text_ == i2.Text_;
	}

	/** @brief Checks whether the activity info structures are not equal.
	 *
	 * Returns true if \em i1 is not equal to \em i2, that is, if at
	 * least one field of \em i1 is not equal to the corresponding one of
	 * \em i2. Otherwise returns false.
	 *
	 * @param[in] i1 The first activity info structure.
	 * @param[in] i2 The second activity info structure.
	 * @return Whether \em i1 and \em i2 are not equal.
	 */
	inline bool operator!= (const ActivityInfo& i1, const ActivityInfo& i2)
	{
		return !(i1 == i2);
	}
}
}

Q_DECLARE_METATYPE (LC::Azoth::ActivityInfo)
