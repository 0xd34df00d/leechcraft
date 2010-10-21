/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "importentityhandler.h"
#include <memory>
#include <QMainWindow>
#include <QProgressDialog>
#include <interfaces/structures.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			ImportEntityHandler::ImportEntityHandler (QObject *parent)
			: QObject (parent)
			{
			}

			void ImportEntityHandler::Import (const Entity& e)
			{
				qDebug () << Q_FUNC_INFO;
				QList<QVariant> history = e.Additional_ ["BrowserHistory"].toList ();
				if (history.size ())
				{
					QProgressDialog progressDia (tr ("Importing history..."),
							tr ("Abort history import"),
							0, history.size (),
							Core::Instance ().GetProxy ()->GetMainWindow ());
					int cur = 0;
					qDebug () << "History:" << history.size ();
					Q_FOREACH (const QVariant& hRowVar, history)
					{
						QMap<QString, QVariant> hRow = hRowVar.toMap ();
						QString title = hRow ["Title"].toString ();
						QString url = hRow ["URL"].toString ();
						QDateTime date = hRow ["DateTime"].toDateTime ();

						if (!date.isValid ())
							qWarning () << "skipping entity with invalid date" << title << url;
						else
							Core::Instance ().GetHistoryModel ()->AddItem (title, url, date);

						progressDia.setValue (++cur);
						if (progressDia.wasCanceled ())
							break;
					}
				}

				QList<QVariant> bookmarks = e.Additional_ ["BrowserBookmarks"].toList ();
				if (bookmarks.size ())
				{
					QProgressDialog progressDia (tr ("Importing bookmarks..."),
							tr ("Abort bookmarks import"),
							0, bookmarks.size (),
							Core::Instance ().GetProxy ()->GetMainWindow ());
					int cur = 0;
					qDebug () << "Bookmarks" << bookmarks.size ();
					Q_FOREACH (const QVariant& hBMVar, bookmarks)
					{
						QMap<QString, QVariant> hBM = hBMVar.toMap ();
						QString title = hBM ["Title"].toString ();
						QString url = hBM ["URL"].toString ();
						QStringList tags = hBM ["Tags"].toStringList ();

						Core::Instance ().GetFavoritesModel ()->AddItem (title, url, tags);
						progressDia.setValue (++cur);
						if (progressDia.wasCanceled ())
							break;
					}
				}
			}
		}
	}
}
