/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>

namespace LC
{
namespace Azoth
{
	struct MoodInfo;

	/** @brief Interface for accounts supporting user mood.
	 * 
	 * This interface can be implemented by account objects to advertise
	 * the support for publishing current user mood.
	 *
	 * @sa IAccount
	 * @sa MoodInfo
	 */
	class ISupportMood
	{
	public:
		virtual ~ISupportMood () {}

		/** @brief Publishes the current user mood.
		 *
		 * @param[in] mood The mood description.
		 */
		virtual void SetMood (const MoodInfo& mood) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::ISupportMood,
		"org.Deviant.LeechCraft.Azoth.ISupportMood/1.0")
