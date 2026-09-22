/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "feedserrormanager.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <util/gui/util.h>
#include <util/xpc/util.h>

namespace LC::Aggregator
{
	namespace
	{
		auto MakeEventId (IDType_t id)
		{
			return "FeedID/" + QString::number (id);
		}
	}

	void FeedsErrorManager::AddFeedError (IDType_t id, const QString& feedName, const Error& error)
	{
		auto& errors = Errors_ [id];
		if (errors.contains (error))
			return;

		errors << error;
		emit gotErrors (id);

		auto e = Util::MakeAN (NotificationTitle,
				tr ("Error updating feed %1: %2.")
					.arg (Util::FormatName (feedName), error.Message_),
				Priority::Warning,
				PluginId,
				AN::CatNews, AN::TypeNewsSourceBroken,
				MakeEventId (id),
				{},
				0, 1,
				error.Message_);
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void FeedsErrorManager::ClearFeedErrors (IDType_t id)
	{
		if (!Errors_.remove (id))
			return;

		emit clearedErrors (id);

		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeANCancel (PluginId, MakeEventId (id)));
	}

	QList<FeedsErrorManager::Error> FeedsErrorManager::GetFeedErrors (IDType_t id) const
	{
		return Errors_.value (id);
	}
}
