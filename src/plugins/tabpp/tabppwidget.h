/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#ifndef PLUGINS_TABPP_TABPPWIDGET_H
#define PLUGINS_TABPP_TABPPWIDGET_H
#include <QDockWidget>
#include "ui_tabppwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			class TabsFilterModel;

			class TabPPWidget : public QDockWidget
			{
				Q_OBJECT

				Ui::TabPPWidget Ui_;
				bool ShouldFloat_;
				TabsFilterModel *TabsFilterModel_;
			public:
				TabPPWidget (const QString&, QWidget* = 0);
				void Release ();

				QTreeView* GetView () const;
				QAction* GetActivatorAction () const;
			protected:
				bool eventFilter (QObject*, QEvent*);
			private slots:
				void handleCustomContextMenuRequested (const QPoint&);
				void handleActivatorHovered ();
				void handleFirstTriggered ();
				void selected (const QModelIndex&);
				void handleDockLocationChanged (Qt::DockWidgetArea);
				void handleTopLevelChanged (bool);
				void handleVisibilityChanged (bool);
			};
		};
	};
};

#endif

