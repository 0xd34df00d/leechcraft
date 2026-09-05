/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "importentity.h"
#include <QMainWindow>
#include <QProgressDialog>
#include <interfaces/structures.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "favoritesmodel.h"

namespace LC::Poshuku
{
	void ImportEntity (const Entity& e,
			FavoritesModel *favoritesModel, IRootWindowsManager *rootWM)
	{
		const auto& bookmarks = e.Additional_ ["BrowserBookmarks"].toList ();
		if (bookmarks.isEmpty ())
			return;

		QProgressDialog progressDia
		{
			QObject::tr ("Importing bookmarks..."),
			QObject::tr ("Abort"),
			0,
			static_cast<int> (bookmarks.size ()),
			rootWM->GetPreferredWindow ()
		};
		for (const auto& hBMVar : bookmarks)
		{
			const auto& hBM = hBMVar.toMap ();
			const auto& title = hBM ["Title"].toString ();
			const auto& url = hBM ["URL"].toString ();
			const auto& tags = hBM ["Tags"].toStringList ();

			favoritesModel->addItem (title, url, tags);

			progressDia.setValue (progressDia.value () + 1);
			if (progressDia.wasCanceled ())
				break;
		}
	}
}
