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
			class TabPPWidget : public QDockWidget
			{
				Q_OBJECT

				Ui::TabPPWidget Ui_;
				bool FirstTime_;
			public:
				TabPPWidget (const QString&, QWidget* = 0);

				QTreeView* GetView () const;
				QAction* GetActivatorAction () const;
			private slots:
				void handleActivatorHovered ();
				void selected (const QModelIndex&);
			};
		};
	};
};

#endif

