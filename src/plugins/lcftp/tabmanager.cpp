/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "tabmanager.h"
#include <QUrl>
#include <QDir>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			TabManager::TabManager (QObject *parent)
			: QObject (parent)
			{
			}

			TabManager::~TabManager ()
			{
				qDeleteAll (Widgets_);
			}

			void TabManager::AddTab (const QUrl& url, QString local)
			{
				if (local.isEmpty () ||
						local == "." ||
						local == "..")
					local = XmlSettingsManager::Instance ()
						.Property ("LastPanedLocalPath", QDir::homePath ()).toString ();

				TabWidget_ptr w (new TabWidget (url, local));
				emit addNewTab (url.host (), w);
				emit changeTabIcon (w, QIcon (":/resources/images/lcftp.svg"));
				Widgets_ << w;
			}

			void TabManager::Remove (TabWidget *tab)
			{
				Widgets_.removeAll (tab);
				emit removeTab (tab);
				tab->deleteLater ();
			}
		};
	};
};

