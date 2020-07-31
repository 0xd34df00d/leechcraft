/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "loadprogressreporter.h"
#include <QtDebug>
#include "application.h"
#include "loadprocess.h"
#include "splashscreen.h"

namespace LC
{
	ILoadProcess_ptr LoadProgressReporter::InitiateProcess (const QString& title, int min, int max)
	{
		const auto& process = std::make_shared<LoadProcess> (title, min, max);

		if (const auto splash = static_cast<Application*> (qApp)->GetSplashScreen ())
			splash->RegisterLoadProcess (process.get ());
		else
			qWarning () << Q_FUNC_INFO
					<< "no splash screen already";

		return process;
	}
}
