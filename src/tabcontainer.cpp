#include "tabcontainer.h"
#include <QTabWidget>
#include "core.h"
#include "xmlsettingsmanager.h"

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

void TabContainer::Add (QWidget *contents, const QString& name,
		const QIcon& icon)
{
	if (TabMode_)
		TabWidget_->addTab (contents, icon, name);
	else
	{
		contents->setWindowFlags (Qt::Window);
		contents->setWindowTitle (name);
		contents->setWindowIcon (icon);
		contents->show ();
		Widgets_.push_front (contents);
	}
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
	else
	{
		Widgets_.removeAll (contents);
		contents->deleteLater ();
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
	else
		contents->setWindowTitle (name);
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
	else
		contents->setWindowIcon (icon);
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
	else
		widget->show ();
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

void TabContainer::ToggleMultiwindow ()
{
	TabMode_ = !TabMode_;
	if (TabMode_)
	{
		while (Widgets_.size ())
		{
			QWidget *widget = Widgets_.takeLast ();
			widget->setWindowFlags (Qt::Widget);
			Add (widget, widget->windowTitle ());
			TabWidget_->setTabIcon (TabWidget_->count () - 1,
					widget->windowIcon ());
		}
	}
	else
	{
		int count = TabWidget_->count ();
		for (int i = count - 1; i >= 2; --i)
		{
			QWidget *widget = TabWidget_->widget (i);
			widget->setWindowTitle (TabWidget_->tabText (i));
			widget->setWindowIcon (TabWidget_->tabIcon (i));
			TabWidget_->removeTab (i);
			widget->setWindowFlags (Qt::Window);
			widget->show ();
			Widgets_ << widget;
		}
	}
}

void TabContainer::handleTabNames ()
{
	int size = Core::Instance ().CountUnremoveableTabs ();
	if (XmlSettingsManager::Instance ()->property ("ShowTabNames").toBool ())
	{
		for (int i = 0; i < size; ++i)
			if (TabWidget_->tabText (i).isNull () ||
					TabWidget_->tabText (i).isEmpty ())
				TabWidget_->setTabText (i, TabNames_ [i]);
	}
	else
	{
		TabNames_.clear ();
		for (int i = 0; i < size; ++i)
		{
			TabNames_ << TabWidget_->tabText (i);
			TabWidget_->setTabText (i, QString ());
		}
	}
}

int TabContainer::FindTabForWidget (QWidget *widget) const
{
	for (int i = 0; i < TabWidget_->count (); ++i)
		if (TabWidget_->widget (i) == widget)
			return i;
	return -1;
}

