/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <memory>
#include <QObject>
#include "feed.h"

namespace LC
{
	struct Entity;
}

namespace LC::Aggregator
{
	class UpdatesManager;
}

namespace LC::Aggregator::Opml
{
	bool IsOpmlEntity (const Entity&);

	void HandleOpmlFile (const QString&, UpdatesManager&);
	void HandleOpmlEntity (const Entity&, std::weak_ptr<UpdatesManager>);
}
