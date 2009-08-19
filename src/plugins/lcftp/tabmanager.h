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

#ifndef PLUGINS_LCFTP_TABMANAGER_H
#define PLUGINS_LCFTP_TABMANAGER_H
#include <QObject>
#include "tabwidget.h"

class QUrl;
class QString;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			class TabManager : public QObject
			{
				Q_OBJECT

				QList<TabWidget_ptr> Widgets_;
			public:
				TabManager (QObject* = 0);
				~TabManager ();

				void AddTab (const QUrl&, QString);
				void Remove (TabWidget*);
			signals:
				void addNewTab (const QString&, QWidget*);
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void statusBarChanged (QWidget*, const QString&);
			};
		};
	};
};

#endif

