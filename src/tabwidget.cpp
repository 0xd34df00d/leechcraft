#include "tabwidget.h"
#include <QTabBar>
#include "core.h"

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

void TabWidget::checkTabMoveAllowed (int from, int to)
{
	if (!AsResult_ && from < Core::Instance ().CountUnremoveableTabs ())
	{
		AsResult_ = true;
		tabBar ()->moveTab (to, from);
	}
	AsResult_ = false;
}

