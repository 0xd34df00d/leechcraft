/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

namespace LC
{
namespace Azoth
{
	struct ActivityInfo;

	/** @brief Interface for contacts announcing their current activity.
	 *
	 * This interface should be implemented by those contact list entries
	 * (ICLEntry objects) that support providing information about their
	 * current activity.
	 *
	 * Different variants of an entry (as per ICLEntry::Variants()) can
	 * have different activities, so exact variant is specified both when
	 * announcing the activity change via the activityChanged() signal
	 * and when retrieving the activity via the GetUserActivity()
	 * function.
	 *
	 * @sa ICLEntry
	 * @sa IHaveContactMood
	 * @sa IHaveContactTune
	 * @sa ActivityInfo
	 */
	class IHaveContactActivity
	{
	public:
		virtual ~IHaveContactActivity () {}

		/** @brief Returns the user activity at the given \em variant.
		 *
		 * If the contact does not announce any activity on this
		 * \em variant, an empty ActivityInfo structure is returned, that
		 * is, with empty ActivityInfo::General_ field.
		 *
		 * @param[in] variant The variant to query
		 * @return The information about the activity of the given
		 * \em variant.
		 */
		virtual ActivityInfo GetUserActivity (const QString& variant) const = 0;
	protected:
		/** @brief Notifies that entry's user activity has changed.
		 *
		 * The actual activity is obtained via the GetUserActivity()
		 * method
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose activity has
		 * changed.
		 */
		virtual void activityChanged (const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveContactActivity,
		"org.LeechCraft.Azoth.IHaveContactActivity/1.0")


