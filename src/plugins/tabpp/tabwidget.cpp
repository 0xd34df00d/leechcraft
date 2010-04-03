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

#include "tabwidget.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			QObject *TabWidget::S_MultiTabsParent_ = 0;

			void TabWidget::SetMultiTabsParent (QObject *parent)
			{
				S_MultiTabsParent_ = parent;
			}

			TabWidget::TabWidget (QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);
				Ui_.View_->setModel (Core::Instance ().GetModel ());
			}

			void TabWidget::Remove ()
			{
				emit removeTab (this);
			}

			QToolBar* TabWidget::GetToolBar () const
			{
				return 0;
			}

			void TabWidget::NewTabRequested ()
			{
				QMetaObject::invokeMethod (S_MultiTabsParent_,
						"newTabRequested");
			}

			QObject* TabWidget::ParentMultiTabs () const
			{
				return S_MultiTabsParent_;
			}

			QList<QAction*> TabWidget::GetTabBarContextMenuActions () const
			{
				return QList<QAction*> ();
			}
		};
	};
};

