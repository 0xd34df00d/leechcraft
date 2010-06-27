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

#include "editorpage.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Popishu
		{
			QObject *EditorPage::S_MultiTabsParent_ = 0;

			EditorPage::EditorPage (QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);
				Ui_.TextEditor_->setAutoIndent (true);
			}

			void EditorPage::SetParentMultiTabs (QObject *parent)
			{
				S_MultiTabsParent_ = parent;
			}

			void EditorPage::Remove ()
			{
				emit removeTab (this);
			}

			QToolBar* EditorPage::GetToolBar () const
			{
				return 0;
			}

			void EditorPage::NewTabRequested ()
			{
				Core::Instance ().NewTabRequested ();
			}

			QObject* EditorPage::ParentMultiTabs () const
			{
				return S_MultiTabsParent_;
			}

			QList<QAction*> EditorPage::GetTabBarContextMenuActions () const
			{
				return QList<QAction*> ();
			}
		};
	};
};
