/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <stdexcept>
#include <QObject>
#include <QPointer>
#include "sllconfig.h"

namespace LC::Util
{
	struct UTIL_SLL_API BadTrackingPointerAccess : std::runtime_error
	{
		BadTrackingPointerAccess ();
	};

	[[noreturn, gnu::cold]]
	UTIL_SLL_API void ReportBadAccess ();

	template<typename T, auto QObjectGetter = &T::GetQObject>
	class TrackingPointer
	{
		T *Ptr_ = nullptr;
		QPointer<const QObject> Guard_;
	public:
		TrackingPointer () = default;

		explicit (false) TrackingPointer (T *ptr)
		: Ptr_ { ptr }
		, Guard_ { ptr ? std::invoke (QObjectGetter, ptr) : nullptr }
		{
		}

		TrackingPointer (const TrackingPointer&) = default;
		TrackingPointer (TrackingPointer&&) = default;
		TrackingPointer& operator= (const TrackingPointer&) = default;
		TrackingPointer& operator= (TrackingPointer&&) = default;
		~TrackingPointer () = default;

		bool operator== (const TrackingPointer& other) const
		{
			return Ptr_ == other.Ptr_;
		}

		explicit operator bool () const
		{
			return Guard_;
		}

		T* UnsafeGet () const
		{
			return Ptr_;
		}

		T* operator-> () const
		{
			if (!Guard_)
				ReportBadAccess ();
			return Ptr_;
		}

		T& operator* () const
		{
			if (!Guard_)
				ReportBadAccess ();
			return *Ptr_;
		}
	};
}
