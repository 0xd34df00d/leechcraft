/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "actionresultreporter.h"
#include <QApplication>
#include <QMessageBox>
#include <interfaces/core/ientitymanager.h>
#include <util/sll/visitor.h>
#include <util/xpc/util.h>

namespace LC::Util
{
	using namespace std::chrono_literals;

	namespace
	{
		QDeadlineTimer GetDeadlineTimer (ActionResultReporter::BackgroundPolicy policy)
		{
			return Visit (policy,
					[] (const ActionResultReporter::TimeoutBackgroundPolicy& policy) { return QDeadlineTimer { policy.Timeout_ }; },
					[] (const auto&) { return QDeadlineTimer { QDeadlineTimer::Forever }; });
		}
	}

	ActionResultReporter::ActionResultReporter (IEntityManager& iem, Config config, QWidget *parent)
	: IEM_ { iem }
	, Context_ { config.Context_ }
	, Priority_ { config.Priority_ }
	, Parent_ { parent }
	, TimerBackground_ { GetDeadlineTimer (config.BackgroundPolicy_) }
	{
		Visit (config.BackgroundPolicy_,
				[] (const TimeoutBackgroundPolicy&) {},
				[this] (const auto&)
				{
					InitialFocus_ = qApp->focusWidget ();
				});
	}

	void ActionResultReporter::operator() (const QString& error)
	{
		const auto isBackground = FocusChanged () || TimerBackground_.hasExpired () || (!Parent_ && HadParent_);

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

	bool ActionResultReporter::FocusChanged () const
	{
		if (!InitialFocus_ || !*InitialFocus_)
			return false;

		const auto curFocus = qApp->focusWidget ();
		return InitialFocus_ != curFocus;
	}
}
