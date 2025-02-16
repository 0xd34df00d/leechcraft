/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "loggingfilter.h"
#include <QLoggingCategory>

namespace LC::Util
{
	struct LoggingFilter::Context
	{
		Categories ToDisable_;
		QHash<QLoggingCategory*, QList<QtMsgType>> Disabled_ {};
		QLoggingCategory::CategoryFilter PreviousFilter_ {};
	};

	QList<LoggingFilter::Context> LoggingFilter::ContextStack_;

	LoggingFilter::LoggingFilter (const Categories& cats)
	{
		ContextStack_.push_back ({ .ToDisable_ = cats });
		ContextStack_.back ().PreviousFilter_ = QLoggingCategory::installFilter (&Filter);
	}

	LoggingFilter::~LoggingFilter ()
	{
		const auto& context = ContextStack_.takeLast ();
		for (const auto& [cat, levels] : context.Disabled_.asKeyValueRange ())
			for (const auto level : levels)
				cat->setEnabled (level, true);

		QLoggingCategory::installFilter (context.PreviousFilter_);
	}

	void LoggingFilter::Filter (QLoggingCategory *category)
	{
		auto& context = ContextStack_.back ();

		if (context.PreviousFilter_)
			context.PreviousFilter_ (category);

		for (const auto level : context.ToDisable_.value (QLatin1String { category->categoryName () }))
			if (category->isEnabled (level))
			{
				category->setEnabled (level, false);
				context.Disabled_ [category] << level;
			}
	}
}
