/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDeadlineTimer>
#include <QPointer>
#include <QString>
#include <interfaces/structures.h>
#include "guiconfig.h"

class QWidget;

class IEntityManager;

namespace LC::Util
{
	class UTIL_GUI_API ActionResultReporter
	{
		IEntityManager& IEM_;
		const QString Context_;

		const Priority Priority_;

		QPointer<QWidget> Parent_;
		const bool HadParent_ = Parent_;

		QDeadlineTimer Timer_;
	public:
		struct Config
		{
			QString Context_;
			Priority Priority_ = Priority::Warning;
			std::optional<std::chrono::milliseconds> BackgroundDelay_ {};
		};

		explicit ActionResultReporter (IEntityManager& iem, Config config, QWidget *parent = nullptr);

		void operator() (const QString&);
	};
}
