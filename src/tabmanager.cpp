/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "tabmanager.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QCursor>
#include <QMenu>
#include <QtDebug>
#include <interfaces/ihavetabs.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "separatetabwidget.h"
#include "mainwindow.h"
#include "newtabmenumanager.h"
#include "separatetabbar.h"

using namespace LeechCraft;

TabManager::TabManager (SeparateTabWidget *tabWidget,
		QObject *parent)
: QObject (parent)
, TabWidget_ (tabWidget)
{
	for (int i = 0; i < TabWidget_->WidgetCount (); ++i)
		OriginalTabNames_ << TabWidget_->TabText (i);

	connect (TabWidget_,
			SIGNAL (tabCloseRequested (int)),
			this,
			SLOT (remove (int)));
	connect (TabWidget_,
			SIGNAL (currentChanged (int)),
			this,
			SLOT (handleCurrentChanged (int)));
	connect (TabWidget_,
			SIGNAL (tabWasMoved (int,int)),
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
	closeAllButCurrent->setProperty ("ActionIcon", "closeallbutcurrent");
	TabWidget_->AddAction2TabBar (closeAllButCurrent);
}

QWidget* TabManager::GetWidget (int position) const
{
	return TabWidget_->Widget (position);
}

QToolBar* TabManager::GetToolBar (int position) const
{
	QWidget *widget = TabWidget_->Widget (position);
	ITabWidget *itw = qobject_cast<ITabWidget*> (widget);
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
		return 0;
}

void TabManager::rotateLeft ()
{
	int index = TabWidget_->CurrentIndex ();
	if (index)
		TabWidget_->setCurrentIndex (index - 1);
	else
		TabWidget_->setCurrentIndex (TabWidget_->WidgetCount () - 1);
}

void TabManager::rotateRight ()
{
	int index = TabWidget_->CurrentIndex ();
	if (index < TabWidget_->WidgetCount () - 1)
		TabWidget_->setCurrentIndex (index + 1);
	else
		TabWidget_->setCurrentIndex (0);
}

void TabManager::navigateToTabNumber ()
{
	int n = sender ()->property ("TabNumber").toInt ();
	if (n >= TabWidget_->WidgetCount ())
		return;
	TabWidget_->setCurrentIndex (n);
}

void TabManager::ForwardKeyboard (QKeyEvent *key)
{
	if (!Events_.contains (key))
	{
		Events_ << key;
		if (TabWidget_->CurrentWidget ())
			QCoreApplication::sendEvent (TabWidget_->CurrentWidget (), key);
	}
	Events_.removeAll (key);
}

void TabManager::add (const QString& name, QWidget *contents)
{
	add (name, contents, QIcon ());
}

void TabManager::add (const QString& name, QWidget *contents,
		const QIcon& srcIcon)
{
	QIcon icon = srcIcon;
	if (icon.isNull ())
	{
		ITabWidget *itw = qobject_cast<ITabWidget*> (contents);
		if (!itw)
		{
			qWarning () << Q_FUNC_INFO
					<< contents
					<< "doesn't implement ITabWidget";
			return;
		}
		icon = itw->GetTabClassInfo ().Icon_;
	}

	if (XmlSettingsManager::Instance ()->
			property ("OpenTabNext").toBool ())
	{
		const int current = TabWidget_->CurrentIndex ();
		OriginalTabNames_.insert (current + 1, name);
		TabWidget_->InsertTab (current + 1,
				contents,
				icon,
				MakeTabName (name));
	}
	else
	{
		OriginalTabNames_ << name;
		TabWidget_->AddTab (contents, icon, MakeTabName (name));
	}
	InvalidateName ();
}

void TabManager::remove (QWidget *contents)
{
	if (!TabWidget_->WidgetCount ())
		return;

	const int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->RemoveTab (tabNumber);
	OriginalTabNames_.removeAt (tabNumber);
	InvalidateName ();

	ITabWidget *itw = qobject_cast<ITabWidget*> (contents);
	if (!itw)
	{
		qWarning () << Q_FUNC_INFO
				<< contents
				<< "doesn't implement ITabWidget";
		return;
	}
	if (itw->GetTabClassInfo ().Features_ & TFSingle)
		Core::Instance ().GetNewTabMenuManager ()->SingleRemoved (itw);
}

void TabManager::remove (int index)
{
	if (!TabWidget_->WidgetCount ())
		return;

	QWidget *widget = TabWidget_->Widget (index);
	ITabWidget *itw =
		qobject_cast<ITabWidget*> (widget);
	if (!itw)
	{
		qWarning () << Q_FUNC_INFO
				<< widget
				<< "doesn't implement ITabWidget";
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
			<< TabWidget_->Widget (index);
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< "failed to ITabWidget::Remove"
			<< TabWidget_->Widget (index);
	}
}

void TabManager::removeByContents (QWidget *contents)
{
	remove (TabWidget_->IndexOf (contents));
}

void TabManager::changeTabName (QWidget *contents, const QString& name)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->SetTabText (tabNumber, MakeTabName (name));
	OriginalTabNames_ [tabNumber] = name;
	InvalidateName ();
}

void TabManager::changeTabIcon (QWidget *contents, const QIcon& icon)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->SetTabIcon (tabNumber, icon);
}

void TabManager::changeTooltip (QWidget *contents, QWidget *tip)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->SetTooltip (tabNumber, tip);
}

void TabManager::handleScrollButtons ()
{
	TabWidget_->TabBar ()->setUsesScrollButtons (XmlSettingsManager::Instance ()->
			property ("UseTabScrollButtons").toBool ());
}

void TabManager::bringToFront (QWidget *widget) const
{
	TabWidget_->setCurrentWidget (widget);
}

void TabManager::handleCurrentChanged (int index)
{
	if (index >= TabWidget_->WidgetCount ())
		return;

	InvalidateName ();

	if (TabWidget_->WidgetCount () != 1)
		Core::Instance ().GetReallyMainWindow ()->RemoveMenus (Menus_);

	ITabWidget *imtw = qobject_cast<ITabWidget*> (TabWidget_->Widget (index));
	if (!imtw)
	{
		qWarning () << Q_FUNC_INFO
				<< TabWidget_->Widget (index)
				<< "doesn't implement ITabWidget";
		return;
	}

	QMap<QString, QList<QAction*> > menus = imtw->GetWindowMenus ();
	Core::Instance ().GetReallyMainWindow ()->AddMenus (menus);
	Menus_ = menus;

	imtw->TabMadeCurrent ();
}

void TabManager::handleMoveHappened (int from, int to)
{
	std::swap (OriginalTabNames_ [from],
			OriginalTabNames_ [to]);
	InvalidateName ();
}

void TabManager::handleCloseAllButCurrent ()
{
	QAction *act = qobject_cast<QAction*> (sender ());
	if (!act)
	{
		qWarning () << Q_FUNC_INFO
			<< "sender is not a QAction*"
			<< sender ();
		return;
	}

	for (int i = TabWidget_->WidgetCount () - 1; i >= 0; --i)
		if (i != TabWidget_->GetLastContextMenuTab ())
			remove (i);
}

int TabManager::FindTabForWidget (QWidget *widget) const
{
	for (int i = 0; i < TabWidget_->WidgetCount (); ++i)
		if (TabWidget_->Widget (i) == widget)
			return i;
	return -1;
}

QString TabManager::MakeTabName (const QString& name) const
{
	int width = TabWidget_->fontMetrics ().averageCharWidth ();
	int numChars = 180 / width;

	QString result = name;
	if (result.size () > numChars + 3)
		result = name.left (numChars) + "...";
	return result;
}

void TabManager::InvalidateName ()
{
	int ci = TabWidget_->CurrentIndex ();
	if (ci >= 0 && ci < TabWidget_->WidgetCount ())
		Core::Instance ().GetReallyMainWindow ()->
			SetAdditionalTitle (OriginalTabNames_.at (ci));
	else
		Core::Instance ().GetReallyMainWindow ()->
			SetAdditionalTitle (QString ());
}

