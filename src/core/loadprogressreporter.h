/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/core/iloadprogressreporter.h>

namespace LC
{
	class LoadProgressReporter : public QObject
							   , public ILoadProgressReporter
	{
	public:
		ILoadProcess_ptr InitiateProcess (const QString&, int, int) override;
	};
}
