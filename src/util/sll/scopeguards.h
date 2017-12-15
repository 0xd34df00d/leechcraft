/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#pragma once

#include <QSettings>
#include "util.h"

namespace LeechCraft
{
namespace Util
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
		settings.beginGroup(group);
		return MakeScopeGuard ([&settings] { settings.endGroup (); });
	}
}
}
