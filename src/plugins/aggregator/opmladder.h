/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <optional>
#include <QObject>
#include "feed.h"

namespace LC
{
	struct Entity;
}

namespace LC::Aggregator
{
	class OpmlAdder : public QObject
	{
	public:
		using AddFeedHandler = std::function<void (QString, QStringList, std::optional<Feed::FeedSettings>)>;
	private:
		const AddFeedHandler AddFeedHandler_;
	public:
		OpmlAdder (const AddFeedHandler&, QObject* = nullptr);

		bool HandleOpmlEntity (const Entity&);

		void StartAddingOpml (const QString&);
	private:
		void ReportError (const QString&) const;
	};

	bool IsOpmlEntity (const Entity&);
}
