/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
namespace Azoth
{
	/** @brief Interface for entries that can report their local time.
	 */
	class IHaveEntityTime
	{
	public:
		virtual ~IHaveEntityTime () {}

		/** @brief Requests updating the entry's local time.
		 */
		virtual void UpdateEntityTime () = 0;
	protected:
		/** @brief Notifies that the entry local time is now known or
		 * has changed.
		 *
		 * This signal may be emitted either as the result of calling
		 * UpdateEntityTime() or when the protocol plugin gets the
		 * updated entity local time during the normal course of actions.
		 *
		 * @note This function is expected to be a signal.
		 */
		virtual void entityTimeUpdated () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Azoth::IHaveEntityTime,
		"org.Deviant.LeechCraft.Azoth.IHaveEntityTime/1.0")
