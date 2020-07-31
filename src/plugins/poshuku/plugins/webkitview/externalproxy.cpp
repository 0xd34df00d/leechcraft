/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "externalproxy.h"
#include <QUrl>
#include <interfaces/structures.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	ExternalProxy::ExternalProxy (IEntityManager* iem, QObject* parent)
	: QObject { parent }
	, IEM_ { iem }
	{
	}

	void ExternalProxy::AddSearchProvider (const QString& url)
	{
		const auto& e = Util::MakeEntity (QUrl { url },
				url,
				FromUserInitiated | OnlyHandle,
				"application/opensearchdescription+xml");
		IEM_->HandleEntity (e);
	}
}
}
}
