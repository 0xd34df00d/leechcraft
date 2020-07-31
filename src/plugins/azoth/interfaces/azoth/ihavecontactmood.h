/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QString;

namespace LC
{
namespace Azoth
{
	struct MoodInfo;

	/** @brief Interface for contacts announcing their current mood.
	 *
	 * This interface should be implemented by those contact list entries
	 * (ICLEntry objects) that support providing information about their
	 * current mood.
	 *
	 * Different variants of an entry (as per ICLEntry::Variants()) can
	 * have different moods, so exact variant is specified both when
	 * announcing the mood change via the moodChanged() signal and when
	 * retrieving the mood via the GetUserMood() function.
	 *
	 * @sa ICLEntry
	 * @sa IHaveContactActivity
	 * @sa IHaveContactTune
	 * @sa MoodInfo
	 */
	class IHaveContactMood
	{
	public:
		virtual ~IHaveContactMood () {}

		/** @brief Returns the user mood for the given \em variant.
		 *
		 * If the contact does not announce any mood on this \em variant,
		 * an empty MoodInfo structure is returned, that is, with empty
		 * MoodInfo::General_ field.
		 *
		 * @param[in] variant The variant to query
		 * @return The information about the mood of the given
		 * \em variant.
		 */
		virtual MoodInfo GetUserMood (const QString& variant) const = 0;
	protected:
		/** @brief Notifies that entry's user mood has changed.
		 *
		 * The actual mood is obtained via the GetUserMood() method.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose mood has
		 * changed.
		 */
		virtual void moodChanged (const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveContactMood,
		"org.LeechCraft.Azoth.IHaveContactMood/1.0")
