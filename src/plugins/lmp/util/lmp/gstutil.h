/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <gst/gst.h>
#include "lmputilconfig.h"

template<typename Key, typename T>
class QMap;
class QString;
class QByteArray;

namespace LC
{
namespace LMP
{
namespace GstUtil
{
	LMP_UTIL_API void AddGhostPad (GstElement *from, GstElement *to, const char *name);

	typedef QMap<QString, QString> TagMap_t;

	LMP_UTIL_API bool ParseTagMessage (GstMessage *msg, TagMap_t& tags, const QString& region);

	LMP_UTIL_API void PerformWProbe (GstPad *srcpad, const std::function<void ()>& functor);

	LMP_UTIL_API void DebugPrintState (GstElement*, GstClockTime = 0.1 * GST_SECOND);

	LMP_UTIL_API QString FixEncoding (const QString& str, const QString& region);

	LMP_UTIL_API const char* GetTeePadTemplateName ();
}
}
}
