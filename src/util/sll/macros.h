/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#define LC_STRINGIZE_(x) #x
#define LC_STRINGIZE(x)  LC_STRINGIZE_(x)
#define LC_PRAGMA(x)     _Pragma(LC_STRINGIZE(x))

#if defined(__clang__)
	#define LC_DIAG_PUSH              LC_PRAGMA(clang diagnostic push)
	#define LC_DIAG_POP               LC_PRAGMA(clang diagnostic pop)
	#define LC_DIAG_IGNORE_DEPRECATED LC_PRAGMA(clang diagnostic ignored "-Wdeprecated-declarations")
#elif defined(__GNUC__)
	#define LC_DIAG_PUSH              LC_PRAGMA(GCC diagnostic push)
	#define LC_DIAG_POP               LC_PRAGMA(GCC diagnostic pop)
	#define LC_DIAG_IGNORE_DEPRECATED LC_PRAGMA(GCC diagnostic ignored "-Wdeprecated-declarations")
#else
	#define LC_DIAG_PUSH
	#define LC_DIAG_POP
	#define LC_DIAG_IGNORE_DEPRECATED
#endif

#define LC_SUPPRESS_DEPRECATED_BEGIN \
	LC_DIAG_PUSH                     \
	LC_DIAG_IGNORE_DEPRECATED

#define LC_SUPPRESS_DEPRECATED_END \
	LC_DIAG_POP
