/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include "sllconfig.h"

namespace LC::Util
{
	/** @brief Executes a given action after a given timeout.
	 *
	 * This class can be used to schedule execution of arbitrary
	 * functions after some arbitrary amount of time.
	 *
	 * The DelayedExecutor objects should be created via <code>new</code>
	 * on heap, and they will delete themselves after the corresponding
	 * action is executed.
	 *
	 * The typical usage is as follows:
	 *	\code
		new Util::DelayedExecutor
		{
			[]
			{
				// body of some lambda
			},
			interval
		};
		\endcode
	 * or
	 *	\code
		new Util::DelayedExecutor
		{
			someCallable,
			interval
		};
	   \endcode
	 */
	class UTIL_SLL_API DelayedExecutor : public QObject
	{
		Q_OBJECT
	public:
		typedef std::function<void ()> Actor_f;
	private:
		const Actor_f Actor_;
	public:
		/** @brief Constructs the delayed executor.
		 *
		 * Schedules the execution of \em action after a given \em
		 * timeout.
		 *
		 * If the \em timeout is 0, the \em action will be executed next
		 * time event loop is run.
		 *
		 * If the \em parent object is passed and it is destroyed before
		 * the delayed executor fires, the \em action will not be
		 * performed. This is useful to avoid execution the \em action if
		 * it depends on some object being alive.
		 *
		 * @param[in] action The action to execute.
		 * @param[in] timeout The timeout before executing the action.
		 * @param[in] parent The parent object of this delayed executor.
		 */
		DelayedExecutor (Actor_f action, int timeout = 0, QObject *parent = nullptr);
	private slots:
		void handleTimeout ();
	};

	inline void ExecuteLater (const DelayedExecutor::Actor_f& actor, int delay = 0)
	{
		new DelayedExecutor { actor, delay };
	}
}
