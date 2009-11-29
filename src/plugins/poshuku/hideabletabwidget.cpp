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

#include "hideabletabwidget.h"
#include <QAction>
#include <QTabBar>
#include <QToolBar>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			HideableTabWidget::HideableTabWidget (QWidget *parent)
			: QWidget (parent)
			{
				Ui_.setupUi (this);

				TabBar_ = new QTabBar;
				TabBar_->setShape (QTabBar::RoundedWest);
				TabBar_->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Expanding);
				connect (TabBar_,
						SIGNAL (currentChanged (int)),
						Ui_.Stack_,
						SLOT (setCurrentIndex (int)));
				connect (Ui_.Stack_,
						SIGNAL (currentChanged (int)),
						TabBar_,
						SLOT (setCurrentIndex (int)));

				QVBoxLayout *tabLay = new QVBoxLayout;
				QToolBar *hc = new QToolBar;
				hc->setFixedWidth (QApplication::fontMetrics ().width ("<"));
				tabLay->addWidget (hc);
				tabLay->addWidget (TabBar_);
				tabLay->setStretch (1, 10);
				tabLay->setContentsMargins (0, 0, 0, 0);
				tabLay->setSizeConstraint (QLayout::SetMinimumSize);
				qobject_cast<QHBoxLayout*> (layout ())->insertLayout (0, tabLay);
			}

			void HideableTabWidget::AddPage (const QString& name, QWidget *widget)
			{
				TabBar_->addTab (name);
				Ui_.Stack_->addWidget (widget);
			}
		};
	};
};

