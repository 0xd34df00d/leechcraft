/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>

typedef struct _GstElement GstElement;
typedef struct _GstMessage GstMessage;
typedef struct _GstBus GstBus;

namespace LC
{
namespace LMP
{
	typedef std::function<int (GstBus*, GstMessage*)> SyncHandler_f;
	typedef std::function<void (GstMessage*)> AsyncHandler_f;

	class ISourceObject;

	class IPath
	{
	public:
		virtual void AddSyncHandler (const SyncHandler_f& handler, QObject *dependent) = 0;
		virtual void AddAsyncHandler (const AsyncHandler_f& handler, QObject *dependent) = 0;

		virtual void InsertElement (GstElement*) = 0;
		virtual void RemoveElement (GstElement*) = 0;

		virtual ISourceObject* GetSourceObject () const = 0;
	};
}
}
