/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef POTORCHUWIDGET_H
#define POTORCHUWIDGET_H

#include <QWidget>
#include <QToolBar>
#include <interfaces/ihavetabs.h>

namespace LeechCraft
{
	namespace Potorchu
	{
		class PotorchuWidget : public QWidget
					, public ITabWidget
		{
			Q_OBJECT
			Q_INTERFACES (ITabWidget)
			QToolBar *ToolBar_;
			static QObject *S_ParentMultiTabs_;
		public:
			PotorchuWidget (QWidget *parent = 0, Qt::WindowFlags f = 0);
			virtual ~PotorchuWidget ();
			static void SetParentMultiTabs (QObject *parent);
			TabClassInfo GetTabClassInfo () const;
			QObject* ParentMultiTabs ();
			void Remove ();
			QToolBar* GetToolBar () const;
		signals:
			void needToClose ();
		};
	}
}

#endif // POTORCHUWIDGET_H
