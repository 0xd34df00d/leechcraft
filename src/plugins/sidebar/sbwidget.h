/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QWidget>
#include <interfaces/core/icoreproxy.h>
#include "ui_sbwidget.h"

class QToolButton;

namespace LeechCraft
{
namespace Util
{
	class FlowLayout;
}

struct TabClassInfo;

namespace Sidebar
{
	class SBWidget : public QWidget
	{
		Q_OBJECT

		Ui::SBWidget Ui_;
		Util::FlowLayout *TrayLay_;

		ICoreProxy_ptr Proxy_;

		const QSize IconSize_;

		QMap<QByteArray, QList<QAction*>> TabClass2Action_;
		QMap<QByteArray, QToolButton*> TabClass2Folder_;
		QMap<QAction*, QWidget*> TabAction2Tab_;

		QMap<QAction*, QToolButton*> CurTab2Button_;
		QMap<QAction*, QToolButton*> TrayAct2Button_;
	public:
		SBWidget (ICoreProxy_ptr, QWidget* = 0);

		void AddTabOpenAction (QAction*);
		void RemoveTabOpenAction (QAction*);

		void AddQLAction (QAction*);
		void RemoveQLAction (QAction*);

		void AddCurTabAction (QAction*, QWidget*);
		void RemoveCurTabAction (QAction*, QWidget*);

		void AddTrayAction (QAction*);
		void RemoveTrayAction (QAction*);
	private:
		QToolButton* AddTabButton (QAction*, QLayout*);
		void RemoveTabButton (QAction*, QLayout*);
		void FoldTabClass (const TabClassInfo&, QAction*);
		void AddToFolder (const QByteArray&, QAction*);
		void UnfoldTabClass (const TabClassInfo&);
	private slots:
		void handleTabContextMenu (const QPoint&);
		void showFolded ();
		void handleTrayActDestroyed ();
	};
}
}
