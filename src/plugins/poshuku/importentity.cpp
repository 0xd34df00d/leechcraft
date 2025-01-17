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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "historymodel.h"
#include "favoritesmodel.h"

namespace LC
{
namespace Poshuku
{
	namespace
	{
		void ImportHistory (const QVariantList& history,
				HistoryModel *historyModel, IRootWindowsManager *rootWM)
		{
			if (history.isEmpty ())
				return;

			QProgressDialog progressDia
			{
				QObject::tr ("Importing history..."),
				QObject::tr ("Abort"),
				0,
				static_cast<int> (history.size ()),
				rootWM->GetPreferredWindow ()
			};
			qDebug () << Q_FUNC_INFO << history.size ();
			for (const QVariant& hRowVar : history)
			{
				const auto& hRow = hRowVar.toMap ();
				const auto& title = hRow ["Title"].toString ();
				const auto& url = hRow ["URL"].toString ();
				const auto& date = hRow ["DateTime"].toDateTime ();

				if (!date.isValid ())
					qWarning () << "skipping entity with invalid date" << title << url;
				else
					historyModel->addItem (title, url, date);

				progressDia.setValue (progressDia.value () + 1);
				if (progressDia.wasCanceled ())
					break;
			}
		}

		void ImportBookmarks (const QVariantList& bookmarks,
				FavoritesModel *favoritesModel, IRootWindowsManager *rootWM)
		{
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
			qDebug () << "Bookmarks" << bookmarks.size ();
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

	void ImportEntity (const Entity& e,
			HistoryModel *historyModel, FavoritesModel *favoritesModel, IRootWindowsManager *rootWM)
	{
		ImportHistory (e.Additional_ ["BrowserHistory"].toList (), historyModel, rootWM);
		ImportBookmarks (e.Additional_ ["BrowserBookmarks"].toList (), favoritesModel, rootWM);
	}
}
}
