/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QStringView>
#include "channel.h"
#include "common.h"

class QDomDocument;
class QUrl;

namespace LC::Aggregator::Parsers
{
	AGGREGATOR_EXPORT bool IsFeedRootName (QStringView rootName);
	AGGREGATOR_EXPORT std::optional<channels_container_t> TryParse (const QDomDocument& doc, IDType_t feedId, const QUrl& feedUrl);
}
