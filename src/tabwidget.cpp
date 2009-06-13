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

void TabWidget::SetTooltip (int index, QWidget *widget)
{
	Widgets_ [index] = widget;
}

bool TabWidget::event (QEvent *e)
{
	if (e->type () == QEvent::ToolTip)
	{
		QHelpEvent *he = static_cast<QHelpEvent*> (e);
		int index = tabBar ()->tabAt (he->pos ());
		if (Widgets_.contains (index) &&
				Widgets_ [index])
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

void TabWidget::tabInserted (int index)
{
	if (index < Core::Instance ().CountUnremoveableTabs ())
		tabBar ()->setTabButton (index, QTabBar::RightSide, 0);
}

void TabWidget::tabRemoved (int index)
{
	Widgets_.remove (index);
	QList<int> keys = Widgets_.keys ();
	for (QList<int>::const_iterator i = keys.begin (),
			end = keys.end (); i != end; ++i)
		if (*i > index)
		{
			Widgets_ [*i - 1] = Widgets_ [*i];
			Widgets_.remove (*i);
		}
}

void TabWidget::checkTabMoveAllowed (int from, int to)
{
	int unrem = Core::Instance ().CountUnremoveableTabs ();
	if (!AsResult_ &&
			(from < unrem || to < unrem))
	{
		AsResult_ = true;
		tabBar ()->moveTab (to, from);
	}
	else
		std::swap (Widgets_ [from], Widgets_ [to]);
	AsResult_ = false;
}

