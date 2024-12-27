/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include "feed.h"

class QDomElement;
class QDomDocument;
class QDomNode;

class ITagsManager;

namespace LC
{
namespace Aggregator
{
	class OPMLWriter
	{
		const ITagsManager * const TagsManager_;
	public:
		explicit OPMLWriter (const ITagsManager*);

		QString Write (const channels_shorts_t&,
				const QString&,
				const QString&,
				const QString&) const;
	private:
		void WriteHead (QDomElement&,
				QDomDocument&,
				const QString&,
				const QString&,
				const QString&) const;
		void WriteBody (QDomElement&,
				QDomDocument&,
				const channels_shorts_t&) const;
	};
}
}
