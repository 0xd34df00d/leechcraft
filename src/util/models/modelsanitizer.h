/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "modelsconfig.h"

class QAbstractItemModel;

namespace LC::Util
{
	/** @brief Attaches a `QAbstractItemModelTester` to `model` for the rest
	 * of its lifetime.
	 *
	 * The tester checks the model against the `QAbstractItemModel`
	 * invariants at the time of the call and after each of the model's
	 * change signals from then on.
	 *
	 * Since the model tester checks invariants on installation time as well
	 * and calls the model's methods, call this only after the class
	 * has set up everything its `QAbstractItemModel` overrides read,
	 * (typically as the last statement of its constructor).
	 * If a class is designed to be derived from and its model-related methods
	 * overridden, never do this from a base class constructor.
	 *
	 * @param[in] model The model to check.
	 *
	 * @ingroup ModelUtil
	 */
	UTIL_MODELS_API void InstallModelSanitizer (QAbstractItemModel& model);
}
