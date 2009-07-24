#include "tabcontainer.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QtDebug>
#include <interfaces/imultitabs.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "tabwidget.h"
#include "mainwindow.h"
#include "tabcontents.h"
#include "tabcontentsmanager.h"

using namespace LeechCraft;

TabContainer::TabContainer (TabWidget *tabWidget,
		QObject *parent)
: QObject (parent)
, TabWidget_ (tabWidget)
{
	for (int i = 0; i < TabWidget_->count (); ++i)
		OriginalTabNames_ << TabWidget_->tabText (i);

	connect (TabWidget_,
			SIGNAL (tabCloseRequested (int)),
			this,
			SLOT (remove (int)));
	connect (TabWidget_,
			SIGNAL (currentChanged (int)),
			this,
			SLOT (handleCurrentChanged (int)));
	connect (TabWidget_,
			SIGNAL (currentChanged (int)),
			this,
			SLOT (handleCurrentChanged (int)));
	connect (TabWidget_,
			SIGNAL (moveHappened (int, int)),
			this,
			SLOT (handleMoveHappened (int, int)));

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
	return TabWidget_->widget (position);
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
	int index = TabWidget_->currentIndex ();
	if (index)
		TabWidget_->setCurrentIndex (index - 1);
	else
		TabWidget_->setCurrentIndex (TabWidget_->count () - 1);
}

void TabContainer::RotateRight ()
{
	int index = TabWidget_->currentIndex ();
	if (index < TabWidget_->count () - 1)
		TabWidget_->setCurrentIndex (index + 1);
	else
		TabWidget_->setCurrentIndex (0);
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
	TabWidget_->addTab (contents, icon, MakeTabName (name));
	OriginalTabNames_ << name;
	InvalidateName ();
}

void TabContainer::remove (QWidget *contents)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->removeTab (tabNumber);
	OriginalTabNames_.removeAt (tabNumber);
	InvalidateName ();
}

void TabContainer::remove (int index)
{
	if (index >= Core::Instance ().CountUnremoveableTabs ())
	{
		QWidget *widget = TabWidget_->widget (index);
		TabContents *tc = qobject_cast<TabContents*> (widget);
		if (tc)
		{
			TabContentsManager::Instance ().RemoveTab (tc);
			return;
		}

		IMultiTabsWidget *itw =
			qobject_cast<IMultiTabsWidget*> (widget);
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
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->setTabText (tabNumber, MakeTabName (name));
	OriginalTabNames_ [tabNumber] = name;
	InvalidateName ();
}

void TabContainer::changeTabIcon (QWidget *contents, const QIcon& icon)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->setTabIcon (tabNumber, icon);
}

void TabContainer::changeTooltip (QWidget *contents, QWidget *tip)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->SetTooltip (tabNumber, tip);
}

void TabContainer::handleTabNames ()
{
	int size = Core::Instance ().CountUnremoveableTabs ();
	if (XmlSettingsManager::Instance ()->property ("ShowTabNames").toBool ())
	{
		for (int i = 0; i < size; ++i)
			if (TabWidget_->tabText (i).isNull () ||
					TabWidget_->tabText (i).isEmpty ())
				TabWidget_->setTabText (i, UnremTabNames_ [i]);
	}
	else
	{
		UnremTabNames_.clear ();
		for (int i = 0; i < size; ++i)
		{
			UnremTabNames_ << TabWidget_->tabText (i);
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
	TabWidget_->setCurrentWidget (widget);
}

void TabContainer::handleCurrentChanged (int)
{
	InvalidateName ();
}

void TabContainer::handleMoveHappened (int from, int to)
{
	std::swap (OriginalTabNames_ [from],
			OriginalTabNames_ [to]);
	InvalidateName ();
}

int TabContainer::FindTabForWidget (QWidget *widget) const
{
	for (int i = 0; i < TabWidget_->count (); ++i)
		if (TabWidget_->widget (i) == widget)
			return i;
	return -1;
}

QString TabContainer::MakeTabName (const QString& name) const
{
	int width = TabWidget_->fontMetrics ().averageCharWidth ();
	int numChars = 180 / width;

	QString result = name;
	if (result.size () > numChars + 3)
		result = name.left (numChars) + "...";
	return result;
}

void TabContainer::InvalidateName ()
{
	Core::Instance ().GetReallyMainWindow ()->
		SetAdditionalTitle (OriginalTabNames_.at (TabWidget_->currentIndex ()));
}

