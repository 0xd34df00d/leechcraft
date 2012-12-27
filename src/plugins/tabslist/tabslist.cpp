/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Georg Rudoy
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

#include "tabslist.h"
#include <QIcon>
#include <QAction>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QMainWindow>
#include <QKeyEvent>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/icoretabwidget.h>

Q_DECLARE_METATYPE (ICoreTabWidget*)

namespace LeechCraft
{
namespace TabsList
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("tabslist");

		ShowList_ = new QAction (tr ("List of tabs"), this);
		ShowList_->setProperty ("ActionIcon", "view-list-details");
		ShowList_->setShortcut (QString ("Ctrl+Shift+L"));
		ShowList_->setProperty ("Action/ID", GetUniqueID () + "_showlist");
		connect (ShowList_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleShowList ()));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TabsList";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "TabsList";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Displays the list of current tabs and allows one to select one of them.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> actions;
		if (aep == ActionsEmbedPlace::QuickLaunch)
			actions << ShowList_;
		return actions;
	}

	namespace
	{
		class ListEventFilter : public QObject
		{
			QList<QToolButton*> AllButtons_;
			QString SearchText_;
		public:
			ListEventFilter (const QList<QToolButton*>& buttons, QObject *parent = 0)
			: QObject (parent)
			, AllButtons_ (buttons)
			{
			}
		protected:
			bool eventFilter (QObject *obj, QEvent *event)
			{
				if (event->type () != QEvent::KeyPress)
					return false;

				auto key = static_cast<QKeyEvent*> (event);
				switch (key->key ())
				{
				case Qt::Key_Escape:
					obj->deleteLater ();
					return true;
				case Qt::Key_Backspace:
					SearchText_.chop (1);
					FocusSearch ();
					return true;
				case Qt::Key_Enter:
				case Qt::Key_Return:
					Q_FOREACH (auto button, AllButtons_)
						if (button->hasFocus ())
							button->animateClick ();
					return true;
				case Qt::Key_Home:
					AllButtons_.first ()->setFocus ();
					break;
				case Qt::Key_End:
					AllButtons_.last ()->setFocus ();
					break;
				default:
					break;
				}

				if (!key->text ().isEmpty ())
				{
					SearchText_ += key->text ();
					FocusSearch ();
					return true;
				}

				return false;
			}
		private:
			void FocusSearch ()
			{
				Q_FOREACH (QToolButton *butt,
						parent ()->findChildren<QToolButton*> ())
					if (butt->property ("OrigText").toString ()
							.startsWith (SearchText_, Qt::CaseInsensitive))
					{
						butt->setFocus ();
						break;
					}
			}
		};
	}

	void Plugin::handleShowList ()
	{
		auto rootWM = Proxy_->GetRootWindowsManager ();

		ICoreTabWidget *tw = rootWM->GetTabWidget (rootWM->GetPreferredWindowIndex ());

		if (tw->WidgetCount () < 2)
			return;

		QWidget *widget = new QWidget (rootWM->GetPreferredWindow (),
				Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		widget->setAttribute (Qt::WA_TranslucentBackground);
		widget->setWindowModality (Qt::ApplicationModal);

		QVBoxLayout *layout = new QVBoxLayout ();
		layout->setSpacing (1);
		layout->setContentsMargins (1, 1, 1, 1);

		const int currentIdx = tw->CurrentIndex ();
		QToolButton *toFocus = 0;
		QList<QToolButton*> allButtons;
		for (int i = 0, count = tw->WidgetCount (); i < count; ++i)
		{
			const QString& origText = tw->TabText (i);
			QString title = QString ("[%1] ").arg (i + 1) + origText;
			if (title.size () > 100)
				title = title.left (100) + "...";
			QAction *action = new QAction (tw->TabIcon (i),
					title, this);
			action->setProperty ("TabIndex", i);
			action->setProperty ("ICTW", QVariant::fromValue<ICoreTabWidget*> (tw));
			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (navigateToTab ()));
			connect (action,
					SIGNAL (triggered ()),
					widget,
					SLOT (deleteLater ()));

			auto button = new QToolButton ();
			button->setDefaultAction (action);
			button->setToolButtonStyle (Qt::ToolButtonTextBesideIcon);
			button->setSizePolicy (QSizePolicy::Expanding,
					button->sizePolicy ().verticalPolicy ());
			button->setProperty ("OrigText", origText);
			layout->addWidget (button);

			if (currentIdx == i)
				toFocus = button;

			allButtons << button;
		}

		widget->installEventFilter (new ListEventFilter (allButtons, widget));
		widget->setLayout (layout);
		layout->update ();
		layout->activate ();

		const QRect& rect = QApplication::desktop ()->
				screenGeometry (rootWM->GetPreferredWindow ());
		QPoint pos = rect.center ();

		const QSize& size = widget->sizeHint () / 2;
		pos -= QPoint (size.width (), size.height ());

		widget->move (pos);
		widget->show ();

		if (toFocus)
			toFocus->setFocus ();
	}

	void Plugin::navigateToTab ()
	{
		const int idx = sender ()->property ("TabIndex").toInt ();
		auto ictw = sender ()->property ("ICTW").value<ICoreTabWidget*> ();
		ictw->setCurrentTab (idx);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_tabslist, LeechCraft::TabsList::Plugin);
