/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#ifdef __clang__
#define LC_THREAD_ANNOTATION(x) __attribute__((x))
#else
#define LC_THREAD_ANNOTATION(x)
#endif

#define GUARDED_BY(m) LC_THREAD_ANNOTATION(guarded_by(m))
