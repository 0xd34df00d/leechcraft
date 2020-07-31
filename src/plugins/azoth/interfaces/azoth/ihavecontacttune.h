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

namespace Media
{
	struct AudioInfo;
}

namespace LC
{
namespace Azoth
{
	/** @brief Interface for contacts announcing their current tune.
	 *
	 * This interface should be implemented by those contact list entries
	 * (ICLEntry objects) that support providing information about their
	 * current tune.
	 *
	 * Different variants of an entry (as per ICLEntry::Variants()) can
	 * have different tunes, so exact variant is specified both when
	 * announcing the tune change via the tuneChanged() signal and when
	 * retrieving the tune via the GetUserTune() function.
	 *
	 * @sa ICLEntry
	 * @sa IHaveContactActivity
	 * @sa IHaveContactMood
	 */
	class IHaveContactTune
	{
	public:
		virtual ~IHaveContactTune () {}

		/** @brief Returns the user tune for the given \em variant.
		 *
		 * If the contact does not announce any tune on this \em variant,
		 * an empty Media::AudioInfo structure is returned.
		 *
		 * @param[in] variant The variant to query
		 * @return The information about the tune of the given
		 * \em variant.
		 */
		virtual Media::AudioInfo GetUserTune (const QString& variant) const = 0;
	protected:
		/** @brief Notifies that entry's current tune has changed.
		 *
		 * The actual tune is obtained via the GetUserTune() method.
		 *
		 * @note This function is expected to be a signal.
		 *
		 * @param[out] variant Variant of the entry whose tune has
		 * changed.
		 */
		virtual void tuneChanged (const QString& variant) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveContactTune,
		"org.LeechCraft.Azoth.IHaveContactTune/1.0")
