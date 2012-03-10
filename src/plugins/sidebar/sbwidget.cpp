/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "sbwidget.h"
#include <QToolButton>
#include <QKeyEvent>
#include <QMenu>
#include <QtDebug>
#include <util/flowlayout.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoretabwidget.h>

Q_DECLARE_METATYPE (QToolButton*);

namespace LeechCraft
{
namespace Sidebar
{
	const int FoldThreshold = 3;

	SBWidget::SBWidget (ICoreProxy_ptr proxy, QWidget *parent)
	: QWidget (parent)
	, TrayLay_ (new Util::FlowLayout (1, 0, 1))
	, Proxy_ (proxy)
	, IconSize_ (QSize (30, 30))
	{
		qRegisterMetaType<QToolButton*> ("QToolButton*");

		Ui_.setupUi (this);
		static_cast<QVBoxLayout*> (layout ())->addLayout (TrayLay_);

		setMaximumWidth (IconSize_.width () + 2);
	}

	void SBWidget::AddTabOpenAction (QAction *act)
	{
		AddTabButton (act, Ui_.PluginButtonsLay_);
	}

	void SBWidget::RemoveTabOpenAction (QAction *act)
	{
		RemoveTabButton (act, Ui_.PluginButtonsLay_);
	}

	void SBWidget::AddQLAction (QAction *act)
	{
		AddTabButton (act, Ui_.QLLay_);
	}

	void SBWidget::RemoveQLAction (QAction *act)
	{
		RemoveTabButton (act, Ui_.QLLay_);
	}

	void SBWidget::AddCurTabAction (QAction *act, QWidget *w)
	{
		ITabWidget *tw = qobject_cast<ITabWidget*> (w);
		const auto& tabClass = tw->GetTabClassInfo ();
		TabClass2Action_ [tabClass.TabClass_] << act;
		TabAction2Tab_ [act] = w;

		if (TabClass2Action_ [tabClass.TabClass_].size () >= FoldThreshold)
			FoldTabClass (tabClass, act);
		else
		{
			auto but = AddTabButton (act, Ui_.TabsLay_);
			CurTab2Button_ [act] = but;
			but->setProperty ("Sidebar/TabPage", QVariant::fromValue<QWidget*> (w));
			but->setContextMenuPolicy (Qt::CustomContextMenu);
			connect (but,
					SIGNAL (customContextMenuRequested (QPoint)),
					this,
					SLOT (handleTabContextMenu (QPoint)));
		}
	}

	void SBWidget::RemoveCurTabAction (QAction *act, QWidget *w)
	{
		ITabWidget *tw = qobject_cast<ITabWidget*> (w);
		const auto& tabClass = tw->GetTabClassInfo ();
		TabClass2Action_ [tabClass.TabClass_].removeAll (act);
		TabAction2Tab_.remove (act);

		delete CurTab2Button_.take (act);

		if (TabClass2Action_ [tabClass.TabClass_].size () < FoldThreshold)
			UnfoldTabClass (tabClass);
	}

	void SBWidget::AddTrayAction (QAction *act)
	{
		connect (act,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleTrayActDestroyed ()));

		auto tb = new QToolButton;
		const int w = maximumWidth () - TrayLay_->margin () * 4;
		tb->setMaximumSize (w, w);
		tb->setIconSize (IconSize_);
		tb->setAutoRaise (true);
		tb->setDefaultAction (act);
		tb->setPopupMode (QToolButton::DelayedPopup);
		TrayAct2Button_ [act] = tb;

		TrayLay_->addWidget (tb);
	}

	void SBWidget::RemoveTrayAction (QAction *act)
	{
		RemoveTabButton (act, TrayLay_);
	}

	QToolButton* SBWidget::AddTabButton (QAction *act, QLayout *lay)
	{
		auto tb = new QToolButton;
		tb->setIconSize (IconSize_);
		tb->setDefaultAction (act);

		lay->addWidget (tb);

		return tb;
	}

	void SBWidget::RemoveTabButton (QAction *act, QLayout *lay)
	{
		for (int i = 0; i < lay->count (); ++i)
		{
			auto tb = qobject_cast<QToolButton*> (lay->itemAt (i)->widget ());
			if (tb && tb->defaultAction () == act)
			{
				tb->deleteLater ();
				lay->removeWidget (tb);
				break;
			}
		}
	}

	void SBWidget::FoldTabClass (const TabClassInfo& tc, QAction *newAct)
	{
		if (!TabClass2Folder_.contains (tc.TabClass_))
		{
			QAction *foldAct = new QAction (tc.VisibleName_, this);
			foldAct->setToolTip (tc.Description_);
			foldAct->setIcon (tc.Icon_);
			foldAct->setProperty ("Sidebar/TabClass", tc.TabClass_);
			connect (foldAct,
					SIGNAL (triggered ()),
					this,
					SLOT (showFolded ()));

			auto tb = new QToolButton;
			tb->setIconSize (IconSize_);
			tb->setDefaultAction (foldAct);
			TabClass2Folder_ [tc.TabClass_] = tb;
			Ui_.TabsLay_->insertWidget (0, tb);

			Q_FOREACH (QAction *act, TabClass2Action_ [tc.TabClass_])
				AddToFolder (tc.TabClass_, act);
		}
		else
			AddToFolder (tc.TabClass_, newAct);
	}

	void SBWidget::AddToFolder (const QByteArray& tabClass, QAction *act)
	{
		delete CurTab2Button_.take (act);
	}

	void SBWidget::UnfoldTabClass (const TabClassInfo& tc)
	{
		if (!TabClass2Folder_.contains (tc.TabClass_))
			return;

		delete TabClass2Folder_.take (tc.TabClass_);

		Q_FOREACH (QAction *act, TabClass2Action_ [tc.TabClass_])
			CurTab2Button_ [act] = AddTabButton (act, Ui_.TabsLay_);
	}

	namespace
	{
		class ListEventFilter : public QObject
		{
			QWidget *List_;
		public:
			ListEventFilter (QWidget *list)
			: List_ (list)
			{
			}
		protected:
			bool eventFilter (QObject*, QEvent *event)
			{
				if (event->type () == QEvent::FocusOut)
				{
					List_->deleteLater ();
					return true;
				}

				if (event->type () == QEvent::KeyRelease &&
						static_cast<QKeyEvent*> (event)->key () == Qt::Key_Escape)
				{
					List_->deleteLater ();
					return true;
				}

				return false;
			}
		};
	}

	void SBWidget::handleTabContextMenu (const QPoint& pos)
	{
		QToolButton *but = qobject_cast<QToolButton*> (sender ());
		QWidget *w = sender ()->property ("Sidebar/TabPage").value<QWidget*> ();

		auto tw = Proxy_->GetTabWidget ();
		auto menu = tw->GetTabMenu (tw->IndexOf (w));
		menu->exec (but->mapToGlobal (pos));
		menu->deleteLater ();
	}

	void SBWidget::showFolded ()
	{
		const auto& tc = sender ()->
				property ("Sidebar/TabClass").toByteArray ();

		QWidget *w = new QWidget (0, Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		w->setWindowOpacity (0.85);
		w->installEventFilter (new ListEventFilter (w));

		auto layout = new QVBoxLayout;
		layout->setSpacing (0);
		layout->setContentsMargins (0, 0, 0, 0);

		Q_FOREACH (QAction *act, TabClass2Action_ [tc])
		{
			auto tb = new QToolButton;
			tb->setIconSize (IconSize_);
			tb->setToolButtonStyle (Qt::ToolButtonTextBesideIcon);
			tb->setDefaultAction (act);
			tb->setAutoRaise (true);
			tb->setSizePolicy (QSizePolicy::Expanding,
					tb->sizePolicy ().verticalPolicy ());
			layout->addWidget (tb);

			QWidget *tabWidget = TabAction2Tab_ [act];
			tb->setProperty ("Sidebar/TabPage", QVariant::fromValue<QWidget*> (tabWidget));
			tb->setContextMenuPolicy (Qt::CustomContextMenu);
			connect (tb,
					SIGNAL (customContextMenuRequested (QPoint)),
					this,
					SLOT (handleTabContextMenu (QPoint)));

			connect (act,
					SIGNAL (triggered ()),
					w,
					SLOT (deleteLater ()),
					Qt::QueuedConnection);
		}

		w->setLayout (layout);
		w->move (QCursor::pos ());
		w->show ();
	}

	void SBWidget::handleTrayActDestroyed ()
	{
		delete TrayAct2Button_.take (static_cast<QAction*> (sender ()));
	}
}
}
