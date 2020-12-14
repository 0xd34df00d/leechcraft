/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include "sysconfig.h"

namespace LC::Util::SysInfo
{
	/** @brief Returns a string of OS name and version joined together.
	 *
	 * @return The name and version of OS running LeechCraft.
	 */
	UTIL_SYS_API QString GetOSName ();

	/** @brief Describes the OS running LeechCraft.
	 */
	struct OSInfo
	{
		/** @brief Describes the CPU architecture of the OS.
		 *
		 * This describes the architecture of the OS, not the machine
		 * itself. Thus, a 32-bit Linux running on a 64-bit CPU will
		 * still be reported as \em x86 instead of \em x86_64.
		 */
		QString Arch_;

		/** @brief The name of the OS, including the distribution.
		 *
		 * Typical values are:
		 * - Gentoo/Linux
		 * - openSUSE 13.1 (Bottle) (x86_64)
		 * - Mac OS X
		 * - Windows
		 *
		 * On non-Linux systems this field typically matches the Flavour_
		 * field. On Linux it also includes the distribution name and
		 * possibly version.
		 *
		 * @sa Flavour_
		 */
		QString Name_;

		/** @brief The full version of the OS.
		 *
		 * This possibly includes the architecture, the OS release and
		 * OS-dependent version components like kernel version on Linux.
		 */
		QString Version_;

		/** @brief The OS flavour, or name of the OS without any
		 * distribution.
		 *
		 * Typical values are:
		 * - Linux
		 * - Mac OS X
		 * - Windows
		 * - FreeBSD
		 *
		 * On non-Linux systems this typically matches the Name_ field.
		 *
		 * @sa Name_
		 */
		QString Flavour_;
	};

	/** @brief Returns more precise information about OS name and version.
	 *
	 * @return A structure OSInfo consisting of OS name, version and other
	 * fields.
	 */
	UTIL_SYS_API OSInfo GetOSInfo ();
}
