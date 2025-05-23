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
#include <util/sll/visitor.h>
#include <util/xpc/downloaderrorstrings.h>
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

		struct ErrorInfo
		{
			QString Short_;
			QString Full_;
		};

		const auto& errInfo = Util::Visit (error,
				[] (const ParseError& e)
					{ return ErrorInfo { tr ("parse error"), tr ("Parse error: ") + e.Message_ }; },
				[] (const IDownload::Error& e)
					{ return ErrorInfo { Util::GetErrorString (e.Type_), e.Message_ }; });

		auto e = Util::MakeAN (NotificationTitle,
				tr ("Error updating feed %1: %2.")
					.arg (Util::FormatName (feedName))
					.arg (errInfo.Short_),
				Priority::Warning,
				PluginId,
				AN::CatNews, AN::TypeNewsSourceBroken,
				MakeEventId (id),
				{},
				0, 1,
				errInfo.Full_);
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
