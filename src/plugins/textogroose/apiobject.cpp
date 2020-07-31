/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "apiobject.h"

namespace LC
{
namespace Textogroose
{
	ApiObject::ApiObject (const Media::LyricsQuery& query, IScript_ptr script)
	: Query_ { query }
	, Script_ { script }
	{
	}

	void ApiObject::handleFinished (const QVariantMap& map)
	{
		handleFinished (QVariantList { map });
	}

	void ApiObject::handleFinished (const QVariantList& varList)
	{
		QList<Media::LyricsResultItem> results;
		for (const auto& var : varList)
		{
			const auto& map = var.toMap ();
			results.append ({ map ["provider"].toString (), map ["lyrics"].toString () });
		}

		emit finished (this, { results });
	}
}
}
