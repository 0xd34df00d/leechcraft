/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QSettings>
#include "util.h"

namespace LC::Util
{
	/** @brief Creates a scope guard that ends the current group on \em settings.
	 *
	 * @param[in] settings The QSettings object whose current group should be
	 * ended on scope exit.
	 * @return The scope guard calling <code>QSettings::endGroup()</code> on the
	 * \em settings object on scope exit.
	 *
	 * @note BeginGroup() is preferable for most use cases.
	 *
	 * @sa BeginGroup()
	 */
	[[nodiscard]] inline auto MakeEndGroupScopeGuard (QSettings& settings)
	{
		return MakeScopeGuard ([&settings] { settings.endGroup (); });
	}

	/** @brief Begins the \em group on \em settings and returns a scope guard
	 * ending that group.
	 *
	 * @param[in] settings The QSettings object.
	 * @param[in] group The name of the group to begin.
	 * @return The scope guard calling <code>QSettings::endGroup()</code> on the
	 * \em settings object on scope exit.
	 *
	 * @sa MakeEndGroupScopeGuard()
	 */
	[[nodiscard]] inline auto BeginGroup (QSettings& settings, const QString& group)
	{
		settings.beginGroup (group);
		return MakeScopeGuard ([&settings] { settings.endGroup (); });
	}
}
