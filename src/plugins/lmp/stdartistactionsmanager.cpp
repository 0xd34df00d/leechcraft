/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stdartistactionsmanager.h"
#include <QQuickWidget>
#include <QQuickItem>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"
#include "previewhandler.h"

namespace LC
{
namespace LMP
{
	StdArtistActionsManager::StdArtistActionsManager (QQuickWidget *view, QObject* parent)
	: QObject { parent }
	{
		connect (view->rootObject (),
				SIGNAL (bookmarkArtistRequested (QString, QString, QString)),
				this,
				SLOT (handleBookmark (QString, QString, QString)));
		connect (view->rootObject (),
				SIGNAL (previewRequested (QString)),
				Core::Instance ().GetPreviewHandler (),
				SLOT (previewArtist (QString)));
		connect (view->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
		connect (view->rootObject (),
				SIGNAL (browseInfo (QString)),
				&Core::Instance (),
				SIGNAL (artistBrowseRequested (QString)));
	}

	void StdArtistActionsManager::handleBookmark (const QString& name, const QString& page, const QString& tags)
	{
		auto e = Util::MakeEntity (tr ("Check out \"%1\"").arg (name),
				{},
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/todo-item");
		e.Additional_ ["TodoBody"] = tags + "<br />" + QString ("<a href='%1'>%1</a>").arg (page);
		e.Additional_ ["Tags"] = QStringList ("music");
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void StdArtistActionsManager::handleLink (const QString& link)
	{
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeEntity (QUrl (link),
					{},
					FromUserInitiated | OnlyHandle));
	}
}
}
