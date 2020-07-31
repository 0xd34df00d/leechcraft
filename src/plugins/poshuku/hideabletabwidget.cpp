/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hideabletabwidget.h"
#include <QAction>
#include <QTabBar>
#include <QToolBar>
#include <QtDebug>

namespace LC
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
		hc->setFixedWidth (fontMetrics ().horizontalAdvance ('<'));
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
	
	QTabBar* HideableTabWidget::GetMainTabBar () const
	{
		return TabBar_;
	}
}
}
