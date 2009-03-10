#include "tabwidget.h"
#include <QTabBar>
#include <QHelpEvent>
#include <QtDebug>
#include "core.h"
#include "3dparty/qxttooltip.h"

/**
 * Portions of this software derived from Qxt &copy; 2009, licensed
 * under the Common Public License, version 1.0, as published by IBM.
 * You should have received a copy of the CPL along with this software.
 */

using namespace LeechCraft;

TabWidget::TabWidget (QWidget *parent)
: QTabWidget (parent)
, AsResult_ (false)
{
	connect (tabBar (),
			SIGNAL (tabMoved (int, int)),
			this,
			SLOT (checkTabMoveAllowed (int, int)));
}

bool TabWidget::event (QEvent *e)
{
	if (e->type () == QEvent::ToolTip)
	{
		QHelpEvent *he = static_cast<QHelpEvent*> (e);
		int index = tabBar ()->tabAt (he->pos ());
		if (Widgets_.contains (index))
		{
			QxtToolTip::show (he->globalPos (), Widgets_ [index], tabBar ());
			return true;
		}
		else
			return false;
	}
	else
		return QTabWidget::event (e);
}

void TabWidget::SetTooltip (int index, QWidget *widget)
{
	Widgets_ [index] = widget;
}

void TabWidget::checkTabMoveAllowed (int from, int to)
{
	if (!AsResult_ && from < Core::Instance ().CountUnremoveableTabs ())
	{
		AsResult_ = true;
		tabBar ()->moveTab (to, from);
	}
	AsResult_ = false;
}

