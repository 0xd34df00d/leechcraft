/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "modelsanitizer.h"

#ifdef WITH_MODEL_SANITIZER
#include <optional>
#include <QAbstractItemModelTester>
#include <QByteArray>
#include <QtDebug>
#endif

namespace LC::Util
{
	void InstallModelSanitizer ([[maybe_unused]] QAbstractItemModel& model)
	{
#ifdef WITH_MODEL_SANITIZER
		static const auto mode = [] -> std::optional<QAbstractItemModelTester::FailureReportingMode>
		{
			const auto& str = qgetenv ("LC_MODEL_SANITIZER_MODE");
			if (str.isEmpty ())
				return {};

			using enum QAbstractItemModelTester::FailureReportingMode;
			if (str == "fatal")
				return Fatal;
			if (str == "warning")
				return Warning;

			qCritical () << "unknown model sanitizer mode" << str;
			return {};
		} ();

		if (mode)
			new QAbstractItemModelTester { &model, *mode, &model };
#endif
	}
}
