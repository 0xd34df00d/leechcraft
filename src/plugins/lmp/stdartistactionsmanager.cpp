/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stdartistactionsmanager.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickWidget>
#include <QQuickItem>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"

namespace LC::LMP
{
	StdArtistActionsManager::StdArtistActionsManager (QQuickWidget& view, QObject* parent)
	: QObject { parent }
	{
		if (const auto ctx = view.engine ()->rootContext ();
			ctx->contextProperty ("stdActions").isNull ())
			ctx->setContextProperty ("stdActions", this);
		else
			deleteLater ();
	}

	void StdArtistActionsManager::bookmarkArtist (const QString& name, const QString& page, const QString& tags)
	{
		auto e = Util::MakeEntity (tr ("Check out \"%1\"").arg (name),
				{},
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/todo-item");
		e.Additional_ ["TodoBody"] = tags + "<br />" + QString ("<a href='%1'>%1</a>").arg (page);
		e.Additional_ ["Tags"] = QStringList ("music");
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void StdArtistActionsManager::browseArtistInfo (const QString& name)
	{
		emit Core::Instance ().artistBrowseRequested (name);
	}

	void StdArtistActionsManager::openLink (const QString& link)
	{
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeEntity (QUrl (link),
					{},
					FromUserInitiated | OnlyHandle));
	}
}
