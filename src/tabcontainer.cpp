#include "tabcontainer.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QtDebug>
#include <interfaces/imultitabs.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "tabwidget.h"

using namespace LeechCraft;

TabContainer::TabContainer (TabWidget *tabWidget,
		QObject *parent)
: QObject (parent)
, TabWidget_ (tabWidget)
, TabMode_ (true)
{
	connect (TabWidget_,
			SIGNAL (tabCloseRequested (int)),
			this,
			SLOT (remove (int)));

	XmlSettingsManager::Instance ()->RegisterObject ("ShowTabNames",
			this, "handleTabNames");
	XmlSettingsManager::Instance ()->RegisterObject ("UseTabScrollButtons",
			this, "handleScrollButtons");

	handleScrollButtons ();
}

TabContainer::~TabContainer ()
{
}

QWidget* TabContainer::GetWidget (int position) const
{
	if (TabMode_)
		return TabWidget_->widget (position);
	else
		return 0;
}

QToolBar* TabContainer::GetToolBar (int position) const
{
	QWidget *widget = TabWidget_->widget (position);
	if (position >= Core::Instance ().CountUnremoveableTabs ())
	{
		IMultiTabsWidget *itw = qobject_cast<IMultiTabsWidget*> (widget);
		if (!itw)
		{
			qWarning () << Q_FUNC_INFO
				<< "casting to ITabWidget* failed for"
				<< TabWidget_->widget (position);
			return 0;
		}
		try
		{
			return itw->GetToolBar ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to ITabWidget::GetToolBar"
				<< e.what ()
				<< widget;
			return 0;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to ITabWidget::GetToolBar"
				<< widget;
			return 0;
		}
	}
	else
		return StaticBars_ [widget];
}

void TabContainer::SetToolBar (QToolBar *bar, QWidget *tw)
{
	StaticBars_ [tw] = bar;
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
			add (widget->windowTitle (), widget);
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

void TabContainer::ForwardKeyboard (QKeyEvent *key)
{
	if (!Events_.contains (key))
	{
		Events_ << key;
		QCoreApplication::sendEvent (TabWidget_->currentWidget (), key);
	}
	Events_.removeAll (key);
}

void TabContainer::add (const QString& name, QWidget *contents)
{
	add (name, contents, QIcon ());
}

void TabContainer::add (const QString& name, QWidget *contents,
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

void TabContainer::remove (QWidget *contents)
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

void TabContainer::remove (int index)
{
	if (index >= Core::Instance ().CountUnremoveableTabs ())
	{
		IMultiTabsWidget *itw =
			qobject_cast<IMultiTabsWidget*> (TabWidget_->widget (index));
		if (!itw)
		{
			qWarning () << Q_FUNC_INFO
				<< "casting to ITabWidget* failed for"
				<< TabWidget_->widget (index);
			return;
		}
		try
		{
			itw->Remove ();
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to ITabWidget::Remove"
				<< e.what ()
				<< TabWidget_->widget (index);
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< "failed to ITabWidget::Remove"
				<< TabWidget_->widget (index);
		}
	}
}

void TabContainer::changeTabName (QWidget *contents, const QString& name)
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

void TabContainer::changeTabIcon (QWidget *contents, const QIcon& icon)
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

void TabContainer::changeTooltip (QWidget *contents, QWidget *tip)
{
	if (TabMode_)
	{
		int tabNumber = FindTabForWidget (contents);
		if (tabNumber == -1)
			return;
		TabWidget_->SetTooltip (tabNumber, tip);
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

void TabContainer::handleScrollButtons ()
{
	TabWidget_->setUsesScrollButtons (XmlSettingsManager::Instance ()->
			property ("UseTabScrollButtons").toBool ());
}

void TabContainer::bringToFront (QWidget *widget) const
{
	if (TabMode_)
		TabWidget_->setCurrentWidget (widget);
	else
		widget->show ();
}

int TabContainer::FindTabForWidget (QWidget *widget) const
{
	for (int i = 0; i < TabWidget_->count (); ++i)
		if (TabWidget_->widget (i) == widget)
			return i;
	return -1;
}

