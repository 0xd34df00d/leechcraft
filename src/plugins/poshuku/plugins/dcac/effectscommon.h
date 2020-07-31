/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtGlobal>

#if defined (Q_PROCESSOR_X86_64) || defined (Q_PROCESSOR_X86)

#define SSE_ENABLED
#include <tmmintrin.h>
#include <immintrin.h>
#include <avxintrin.h>
#include <avx2intrin.h>

#endif
