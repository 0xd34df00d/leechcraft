/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "actionresultreporter.h"
#include <QMessageBox>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>

namespace LC::Util
{
	using namespace std::chrono_literals;

	ActionResultReporter::ActionResultReporter (IEntityManager& iem, Config config, QWidget *parent)
	: IEM_ { iem }
	, Context_ { config.Context_ }
	, Priority_ { config.Priority_ }
	, Parent_ { parent }
	, Timer_ { config.BackgroundDelay_.value_or (5s) }
	{
	}

	void ActionResultReporter::operator() (const QString& error)
	{
		const auto isBackground = Timer_.hasExpired () || (!Parent_ && HadParent_);

		if (isBackground)
			IEM_.HandleEntity (MakeNotification (Context_, error, Priority_));
		else
			switch (Priority_)
			{
			case Priority::Info:
				QMessageBox::information (Parent_, Context_, error);
				break;
			case Priority::Warning:
				QMessageBox::warning (Parent_, Context_, error);
				break;
			case Priority::Critical:
				QMessageBox::critical (Parent_, Context_, error);
				break;
			}
	}
}
