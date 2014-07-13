/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
#include "mainwindowmenumanager.h"

using namespace LeechCraft;

TabManager::TabManager (SeparateTabWidget *tabWidget, MainWindow *window, QObject *parent)
: QObject (parent)
, TabWidget_ (tabWidget)
, Window_ (window)
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
			SIGNAL (tabWasMoved (int, int)),
			this,
			SLOT (handleMoveHappened (int, int)));

	QAction *closeAllButCurrent = new QAction (tr ("Close all but this"),
			this);
	connect (closeAllButCurrent,
			SIGNAL (triggered ()),
			this,
			SLOT (handleCloseAllButCurrent ()));
	closeAllButCurrent->setProperty ("ActionIcon", "tab-close-other");
	TabWidget_->AddAction2TabBar (closeAllButCurrent);
}

QWidget* TabManager::GetCurrentWidget () const
{
	return TabWidget_->CurrentWidget ();
}

QWidget* TabManager::GetWidget (int position) const
{
	return TabWidget_->Widget (position);
}

int TabManager::GetWidgetCount () const
{
	return TabWidget_->WidgetCount ();
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
		TabWidget_->setCurrentTab (index - 1);
	else
		TabWidget_->setCurrentTab (TabWidget_->WidgetCount () - 1);
}

void TabManager::rotateRight ()
{
	int index = TabWidget_->CurrentIndex ();
	if (index < TabWidget_->WidgetCount () - 1)
		TabWidget_->setCurrentTab (index + 1);
	else
		TabWidget_->setCurrentTab (0);
}

void TabManager::navigateToTabNumber ()
{
	int n = sender ()->property ("TabNumber").toInt ();
	if (n >= TabWidget_->WidgetCount ())
		return;

	TabWidget_->setCurrentTab (n);
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
		QIcon icon)
{
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

		if (itw->GetTabClassInfo ().Features_ & TFSingle)
			Core::Instance ().GetNewTabMenuManager ()->HideAction (itw);
	}

	if (XmlSettingsManager::Instance ()->
			property ("OpenTabNext").toBool ())
	{
		const int current = TabWidget_->CurrentIndex ();
		OriginalTabNames_.insert (current + 1, name);
		TabWidget_->InsertTab (current + 1,
				contents,
				icon,
				name);
	}
	else
	{
		OriginalTabNames_ << name;
		TabWidget_->AddTab (contents, icon, name);
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
	const auto& features = itw->GetTabClassInfo ().Features_;
	if (features & TFSingle)
		Core::Instance ().GetNewTabMenuManager ()->SingleRemoved (itw);
	if (features & TFByDefault)
		Core::Instance ().GetNewTabMenuManager ()->ToggleHide (itw, true);
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
	TabWidget_->SetTabText (tabNumber, name);
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

void TabManager::bringToFront (QWidget *widget) const
{
	if (TabWidget_->IndexOf (widget) != -1)
	{
		TabWidget_->setCurrentWidget (widget);
		Window_->show ();
		Window_->activateWindow ();
	}
}

void TabManager::handleCurrentChanged (int index)
{
	if (index >= TabWidget_->WidgetCount ())
		return;

	InvalidateName ();

	if (auto prevTab = TabWidget_->GetPreviousWidget ())
		if (auto imtw = qobject_cast<ITabWidget*> (prevTab))
			imtw->TabLostCurrent ();

	if (TabWidget_->WidgetCount () != 1)
		Window_->GetMenuManager ()->RemoveMenus (Menus_);

	auto tab = TabWidget_->Widget (index);
	auto imtw = qobject_cast<ITabWidget*> (tab);
	if (!imtw)
	{
		qWarning () << Q_FUNC_INFO
				<< TabWidget_->Widget (index)
				<< "doesn't implement ITabWidget";
		return;
	}

	const auto menus = imtw->GetWindowMenus ();
	Window_->GetMenuManager ()->AddMenus (menus);
	Menus_ = menus;

	emit currentTabChanged (tab);

	imtw->TabMadeCurrent ();
}

void TabManager::handleMoveHappened (int from, int to)
{
	OriginalTabNames_.move (from, to);
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
	return TabWidget_->IndexOf (widget);
}

void TabManager::InvalidateName ()
{
	int ci = TabWidget_->CurrentIndex ();
	if (ci >= 0 && ci < TabWidget_->WidgetCount ())
		Window_->SetAdditionalTitle (OriginalTabNames_.at (ci));
	else
		Window_->SetAdditionalTitle (QString ());
}

QStringList TabManager::GetOriginalNames () const
{
	return OriginalTabNames_;
}

void TabManager::SetOriginalNames (const QStringList& names)
{
	OriginalTabNames_ = names;
}
