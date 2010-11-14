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

#ifndef PLUGINS_AZOTH_MAINWIDGET_H
#define PLUGINS_AZOTH_MAINWIDGET_H
#include <QWidget>
#include "ui_mainwidget.h"

class QToolBar;
class QMenu;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			class SortFilterProxyModel;

			class MainWidget : public QWidget
			{
				Q_OBJECT

				Ui::MainWidget Ui_;

				QToolBar *UpperBar_;
				QMenu *MenuGeneral_;
				QMenu *MenuNewAccount_;
				SortFilterProxyModel *ProxyModel_;
			public:
				MainWidget (QWidget* = 0);

				void AddAccountCreators (const QList<QAction*>&);
				void AddMUCJoiners (const QList<QAction*>&);
			private slots:
				void on_CLTree__activated (const QModelIndex&);
			};
		}
	}
}

#endif
