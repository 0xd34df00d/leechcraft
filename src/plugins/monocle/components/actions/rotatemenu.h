/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include "common.h"

class QMenu;

namespace LC::Monocle
{
	struct InitAngle
	{
		double Value_;
	};

	template<typename T>
	using AngleNotifierSignal = void (T::*) (double);

	template<typename T>
	struct AngleNotifier
	{
		T& Obj_;
		AngleNotifierSignal<T> Signal_;
	};

	using AngleGetter = std::variant<InitAngle, AngleNotifier<class PagesLayoutManager>>;

	std::unique_ptr<QMenu> CreateRotateMenu (AngleGetter angleGetter,
			const std::function<void (double, RotationChange)>& angleSetter);
}
