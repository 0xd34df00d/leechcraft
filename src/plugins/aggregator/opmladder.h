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
#include <interfaces/core/icoreproxyfwd.h>
#include "feed.h"

namespace LC
{
	struct Entity;
}

namespace LC::Aggregator
{
	class OpmlAdder : public QObject
	{
		const ICoreProxy_ptr Proxy_;
	public:
		using AddFeedHandler = std::function<void (QString, QStringList, std::optional<Feed::FeedSettings>)>;
	private:
		const AddFeedHandler AddFeedHandler_;
	public:
		OpmlAdder (const AddFeedHandler&, const ICoreProxy_ptr&, QObject* = nullptr);

		bool IsOpmlEntity (const Entity&) const;
		bool HandleOpmlEntity (const Entity&);

		void StartAddingOpml (const QString&);
	private:
		void ReportError (const QString&) const;
	};
}
