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

#ifndef PLUGINS_POPISHU_EDITORPAGE_H
#define PLUGINS_POPISHU_EDITORPAGE_H
#include <QWidget>
#include <interfaces/imultitabs.h>
#include "ui_editorpage.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Popishu
		{
			class EditorPage : public QWidget
							 , public IMultiTabsWidget
			{
				Q_OBJECT
				Q_INTERFACES (IMultiTabsWidget)

				Ui::EditorPage Ui_;

				static QObject* S_MultiTabsParent_;
			public:
				EditorPage (QWidget* = 0);

				static void SetParentMultiTabs (QObject*);

				void Remove ();
				QToolBar* GetToolBar () const;
				void NewTabRequested ();
				QObject* ParentMultiTabs () const;
				QList<QAction*> GetTabBarContextMenuActions () const;
			signals:
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
			};
		};
	};
};

#endif
