/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

/** @brief Interface for plugins wishing to get notifications about the
 * shutdown process.
 */
class Q_DECL_EXPORT IShutdownListener
{
public:
	virtual ~IShutdownListener () {}

	/** @brief This function is called by the LeechCraft Core to notify
	 * the plugin that LeechCraft is shutting down.
	 */
	virtual void HandleShutdownInitiated () = 0;
};

Q_DECLARE_INTERFACE (IShutdownListener, "org.Deviant.LeechCraft.IShutdownListener/1.0")
