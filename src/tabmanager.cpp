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

#include "tabmanager.h"
#include <QCoreApplication>
#include <QKeyEvent>
#include <QCursor>
#include <QMenu>
#include <QtDebug>
#include <interfaces/imultitabs.h>
#include <interfaces/iembedtab.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "tabwidget.h"
#include "mainwindow.h"

using namespace LeechCraft;

TabManager::TabManager (TabWidget *tabWidget,
		QObject *parent)
: QObject (parent)
, TabWidget_ (tabWidget)
, NewTabMenu_ (new QMenu (tr ("New tab menu")))
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
	closeAllButCurrent->setProperty ("ActionIcon", "closeallbutcurrent");
	TabWidget_->AddAction2TabBar (closeAllButCurrent);
}

QWidget* TabManager::GetWidget (int position) const
{
	return TabWidget_->widget (position);
}

QToolBar* TabManager::GetToolBar (int position) const
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

QMenu* TabManager::GetNewTabMenu () const
{
	return NewTabMenu_;
}

void TabManager::SetToolBar (QToolBar *bar, QWidget *tw)
{
	StaticBars_ [tw] = bar;
}

void TabManager::rotateLeft ()
{
	int index = TabWidget_->currentIndex ();
	if (index)
		TabWidget_->setCurrentIndex (index - 1);
	else
		TabWidget_->setCurrentIndex (TabWidget_->count () - 1);
}

void TabManager::rotateRight ()
{
	int index = TabWidget_->currentIndex ();
	if (index < TabWidget_->count () - 1)
		TabWidget_->setCurrentIndex (index + 1);
	else
		TabWidget_->setCurrentIndex (0);
}

void TabManager::navigateToTabNumber ()
{
	int n = sender ()->property ("TabNumber").toInt ();
	if (n >= TabWidget_->count ())
		return;
	TabWidget_->setCurrentIndex (n);
}

void TabManager::ForwardKeyboard (QKeyEvent *key)
{
	if (!Events_.contains (key))
	{
		Events_ << key;
		if (TabWidget_->currentWidget ())
			QCoreApplication::sendEvent (TabWidget_->currentWidget (), key);
	}
	Events_.removeAll (key);
}

void TabManager::AddObject (QObject *obj)
{
	IInfo *ii = qobject_cast<IInfo*> (obj);

	IEmbedTab *iet = qobject_cast<IEmbedTab*> (obj);
	if (iet)
	{
		try
		{
			QString name = ii->GetName ();
			QIcon icon = ii->GetIcon ();
			QToolBar *tb = iet->GetToolBar ();
			QWidget *contents = iet->GetTabContents ();

			if (!EmbedTabs_.contains (contents))
			{
				EmbedTabs_ [contents] = obj;

				add (name,
						contents,
						icon);
				SetToolBar (tb,
						contents);

				if (XmlSettingsManager::Instance ()->
						Property (QString ("Hide%1").arg (name), false).toBool ())
					remove (TabWidget_->indexOf (contents));
			}
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ()
				<< obj;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< obj;
		}
	}

	IMultiTabs *imt = qobject_cast<IMultiTabs*> (obj);
	if (imt && !RegisteredMultiTabs_.contains (obj))
	{
		RegisteredMultiTabs_ << obj;
		try
		{
			QString name = ii->GetName ();
			QString info = ii->GetInfo ();
			QIcon icon = ii->GetIcon ();
			NewTabMenu_->addAction (icon,
					name,
					obj,
					SLOT (newTabRequested ()))->setToolTip (info);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ()
				<< obj;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< obj;
		}
	}
}

void TabManager::add (const QString& name, QWidget *contents)
{
	add (name, contents, QIcon ());
}

void TabManager::add (const QString& name, QWidget *contents,
		const QIcon& icon)
{
	if (XmlSettingsManager::Instance ()->
			property ("OpenTabNext").toBool ())
	{
		int current = TabWidget_->currentIndex ();
		OriginalTabNames_.insert (current + 1, name);
		TabWidget_->insertTab (current + 1,
				contents,
				icon,
				MakeTabName (name));
	}
	else
	{
		OriginalTabNames_ << name;
		TabWidget_->addTab (contents, icon, MakeTabName (name));
	}
	InvalidateName ();

	TabWidget_->setTabsClosable (TabWidget_->count () != 1);
}

void TabManager::remove (QWidget *contents)
{
	if (TabWidget_->count () == 1)
		return;

	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->removeTab (tabNumber);
	OriginalTabNames_.removeAt (tabNumber);
	InvalidateName ();

	TabWidget_->setTabsClosable (TabWidget_->count () != 1);
}

void TabManager::remove (int index)
{
	if (TabWidget_->count () == 1)
		return;

	QWidget *widget = TabWidget_->widget (index);
	IMultiTabsWidget *itw =
		qobject_cast<IMultiTabsWidget*> (widget);
	if (EmbedTabs_.contains (widget))
	{
		QObject *obj = EmbedTabs_ [widget];

		IInfo *ii = qobject_cast<IInfo*> (obj);
		try
		{
			QString name = ii->GetName ();
			QIcon icon = ii->GetIcon ();

			XmlSettingsManager::Instance ()->
					setProperty (qPrintable (QString ("Hide%1").arg (name)),
							true);
			EmbedTabs_.remove (widget);
			remove (widget);

			QAction *action = 0;
			Q_FOREACH (QAction *act, NewTabMenu_->actions ())
				if (act->text () == name)
				{
					action = new QAction (icon, name, this);
					connect (action,
							SIGNAL (triggered ()),
							this,
							SLOT (restoreEmbedTab ()));
					NewTabMenu_->insertAction (act, action);
					NewTabMenu_->removeAction (act);
					ReaddOnRestore_ [name] = act;
					break;
				}

			if (!action)
				action = NewTabMenu_->addAction (icon, name,
						this,
						SLOT (restoreEmbedTab ()));
			action->setData (QVariant::fromValue<QObject*> (obj));
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
				<< e.what ()
				<< obj;
		}
		catch (...)
		{
			qWarning () << Q_FUNC_INFO
				<< obj;
		}
	}
	else if (itw)
	{
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

void TabManager::changeTabName (QWidget *contents, const QString& name)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->setTabText (tabNumber, MakeTabName (name));
	OriginalTabNames_ [tabNumber] = name;
	InvalidateName ();
}

void TabManager::changeTabIcon (QWidget *contents, const QIcon& icon)
{
	int tabNumber = FindTabForWidget (contents);
	if (tabNumber == -1)
		return;
	TabWidget_->setTabIcon (tabNumber, icon);
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
	TabWidget_->setUsesScrollButtons (XmlSettingsManager::Instance ()->
			property ("UseTabScrollButtons").toBool ());
}

void TabManager::bringToFront (QWidget *widget) const
{
	TabWidget_->setCurrentWidget (widget);
}

void TabManager::handleCurrentChanged (int index)
{
	InvalidateName ();

	Core::Instance ().GetReallyMainWindow ()->RemoveMenus (Menus_);

	IMultiTabsWidget *imtw = qobject_cast<IMultiTabsWidget*> (TabWidget_->widget (index));
	if (imtw)
	{
		QMap<QString, QList<QAction*> > menus = imtw->GetWindowMenus ();
		Core::Instance ().GetReallyMainWindow ()->AddMenus (menus);
		Menus_ = menus;
	}
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

	int cur = TabWidget_->TabAt (act->data ().value<QPoint> ());
	for (int i = TabWidget_->count () - 1; i >= 0; --i)
		if (i != cur)
			remove (i);
}

void TabManager::restoreEmbedTab ()
{
	QAction *action = qobject_cast<QAction*> (sender ());
	if (!action)
	{
		qWarning () << Q_FUNC_INFO
			<< "null action, damn"
			<< sender ();
		return;
	}

	QObject *obj = action->data ().value<QObject*> ();
	if (!obj)
	{
		qWarning () << Q_FUNC_INFO
			<< "action's data is not a QObject*"
			<< action
			<< action->data ();
		return;
	}

	IInfo *ii = qobject_cast<IInfo*> (obj);
	try
	{
		QString name = ii->GetName ();
		QIcon icon = ii->GetIcon ();

		XmlSettingsManager::Instance ()->
				setProperty (qPrintable (QString ("Hide%1").arg (name)),
						false);
		if (ReaddOnRestore_.contains (action->text ()))
		{
			QAction *readd = ReaddOnRestore_ [action->text ()];
			NewTabMenu_->insertAction (action, readd);
		}

		action->deleteLater ();
		AddObject (obj);
	}
	catch (const std::exception& e)
	{
		qWarning () << Q_FUNC_INFO
			<< e.what ()
			<< obj;
	}
	catch (...)
	{
		qWarning () << Q_FUNC_INFO
			<< obj;
	}
}

int TabManager::FindTabForWidget (QWidget *widget) const
{
	for (int i = 0; i < TabWidget_->count (); ++i)
		if (TabWidget_->widget (i) == widget)
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
	int ci = TabWidget_->currentIndex ();
	if (ci >= 0)
		Core::Instance ().GetReallyMainWindow ()->
			SetAdditionalTitle (OriginalTabNames_.at (ci));
	else
		Core::Instance ().GetReallyMainWindow ()->
			SetAdditionalTitle (QString ());
}

