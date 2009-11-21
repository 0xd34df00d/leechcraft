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

#include "tabcontainer.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QCursor>
#include <QtDebug>
#include <interfaces/imultitabs.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "tabwidget.h"
#include "mainwindow.h"

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

	XmlSettingsManager::Instance ()->RegisterObject ("UseTabScrollButtons",
			this, "handleScrollButtons");

	handleScrollButtons ();

	QAction *closeAllButCurrent = new QAction (tr ("Close all but this"),
			this);
	connect (closeAllButCurrent,
			SIGNAL (triggered ()),
			this,
			SLOT (handleCloseAllButCurrent ()));
	closeAllButCurrent->setProperty ("ActionIcon", "closeallbutcorrent");
	TabWidget_->AddAction2TabBar (closeAllButCurrent);
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
	IMultiTabsWidget *itw = qobject_cast<IMultiTabsWidget*> (widget);
	if (itw)
	{
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
	OriginalTabNames_ << name;
	if (XmlSettingsManager::Instance ()->
			property ("OpenTabNext").toBool ())
	{
		int current = TabWidget_->currentIndex ();
		TabWidget_->insertTab (current + 1,
				contents,
				icon,
				MakeTabName (name));
	}
	else
		TabWidget_->addTab (contents, icon, MakeTabName (name));
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
	QWidget *widget = TabWidget_->widget (index);
	if (widget->property ("IsUnremoveable").toBool ())
		return;

	IMultiTabsWidget *itw =
		qobject_cast<IMultiTabsWidget*> (widget);
	if (!itw)
		return;

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

void TabContainer::handleCloseAllButCurrent ()
{
	QAction *act = qobject_cast<QAction*> (sender ());
	if (!act)
	{
		qWarning () << Q_FUNC_INFO
			<< "sender is not a QAction*"
			<< sender ();
		return;
	}

	int cur = TabWidget_->TabAt (act->data ().value<QPoint> ());
	for (int i = TabWidget_->count () - 1; i >= 0; --i)
		if (i != cur)
			remove (i);
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
	int ci = TabWidget_->currentIndex ();
	if (ci >= 0)
		Core::Instance ().GetReallyMainWindow ()->
			SetAdditionalTitle (OriginalTabNames_.at (ci));
	else
		Core::Instance ().GetReallyMainWindow ()->
			SetAdditionalTitle (QString ());
}

