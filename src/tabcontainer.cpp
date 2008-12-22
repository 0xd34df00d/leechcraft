#include "tabcontainer.h"
#include <QTabWidget>
#include <QIcon>
#include "core.h"

using namespace LeechCraft;

TabContainer::TabContainer (QTabWidget *tabWidget,
		QObject *parent)
: QObject (parent)
, TabWidget_ (tabWidget)
, TabMode_ (true)
{
}

TabContainer::~TabContainer ()
{
}

void TabContainer::Add (QWidget *contents, const QString& name)
{
	if (TabMode_)
		TabWidget_->addTab (contents, name);
}

void TabContainer::Remove (QWidget *contents)
{
	if (TabMode_)
	{
		int tabNumber = FindTabForWidget (contents);
		if (tabNumber == -1)
			return;
		TabWidget_->removeTab (tabNumber);
	}
}

void TabContainer::ChangeTabName (QWidget *contents, const QString& name)
{
	if (TabMode_)
	{
		int tabNumber = FindTabForWidget (contents);
		if (tabNumber == -1)
			return;
		TabWidget_->setTabText (tabNumber, name);
	}
}

void TabContainer::ChangeTabIcon (QWidget *contents, const QIcon& icon)
{
	if (TabMode_)
	{
		int tabNumber = FindTabForWidget (contents);
		if (tabNumber == -1)
			return;
		TabWidget_->setTabIcon (tabNumber, icon);
	}
}

QWidget* TabContainer::GetWidget (int position) const
{
	if (TabMode_)
		return TabWidget_->widget (position);
	else
		return 0;
}

void TabContainer::BringToFront (QWidget *widget) const
{
	if (TabMode_)
		TabWidget_->setCurrentWidget (widget);
}

bool TabContainer::RemoveCurrent ()
{
	if (TabMode_)
	{
		int index = TabWidget_->currentIndex ();
		if (index < Core::Instance ().CountUnremoveableTabs ())
			return false;
		else
		{
			QWidget *contents = GetWidget (index);
			if (!contents)
				return false;
			Remove (contents);
			contents->deleteLater ();
			return true;
		}
	}
	return false;
}

void TabContainer::RotateLeft ()
{
	if (TabMode_)
	{
		int index = TabWidget_->currentIndex ();
		if (index)
			TabWidget_->setCurrentIndex (index - 1);
		else
			TabWidget_->setCurrentIndex (TabWidget_->count () - 1);
	}
}

void TabContainer::RotateRight ()
{
	if (TabMode_)
	{
		int index = TabWidget_->currentIndex ();
		if (index < TabWidget_->count () - 1)
			TabWidget_->setCurrentIndex (index + 1);
		else
			TabWidget_->setCurrentIndex (0);
	}
}

void TabContainer::SetTabMode (bool tabMode)
{
	TabMode_ = tabMode;
}

int TabContainer::FindTabForWidget (QWidget *widget) const
{
	for (int i = 0; i < TabWidget_->count (); ++i)
		if (TabWidget_->widget (i) == widget)
			return i;
	return -1;
}

