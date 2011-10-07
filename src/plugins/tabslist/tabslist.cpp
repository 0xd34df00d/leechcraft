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
#include <interfaces/core/icoretabwidget.h>

namespace LeechCraft
{
namespace TabsList
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("tabslist");

		ShowList_ = new QAction (tr ("List of tabs"),
				Proxy_->GetMainWindow ());
		ShowList_->setProperty ("ActionIcon", "itemlist");
		ShowList_->setShortcut (QString ("Ctrl+Shift+L"));
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
		if (aep == AEPQuickLaunch)
			actions << ShowList_;
		return actions;
	}

	namespace
	{
		class ListEventFilter : public QObject
		{
			QString SearchText_;
		public:
			ListEventFilter (QObject *parent = 0)
			: QObject (parent)
			{
			}
		protected:
			bool eventFilter (QObject *obj, QEvent *event)
			{
				if (event->type () != QEvent::KeyPress)
					return false;

				QKeyEvent *key = static_cast<QKeyEvent*> (event);
				if (key->key () == Qt::Key_Escape)
				{
					obj->deleteLater ();
					return true;
				}
				else if (key->key () == Qt::Key_Backspace)
				{
					SearchText_.chop (1);
					FocusSearch ();
					return true;
				}
				else if (!key->text ().isEmpty ())
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
		ICoreTabWidget *tw = Proxy_->GetTabWidget ();

		if (tw->WidgetCount () < 2)
			return;

		QWidget *widget = new QWidget (Proxy_->GetMainWindow (),
				Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		widget->setAttribute (Qt::WA_TranslucentBackground);
		widget->installEventFilter (new ListEventFilter (widget));
		widget->setWindowModality (Qt::ApplicationModal);

		QVBoxLayout *layout = new QVBoxLayout ();
		layout->setSpacing (1);
		layout->setContentsMargins (1, 1, 1, 1);

		const int currentIdx = tw->CurrentIndex ();
		QToolButton *toFocus = 0;
		for (int i = 0, count = tw->WidgetCount (); i < count; ++i)
		{
			const QString& origText = tw->TabText (i);
			QString title = QString ("[%1] ").arg (i + 1) + origText;
			if (title.size () > 100)
				title = title.left (100) + "...";
			QAction *action = new QAction (tw->TabIcon (i),
					title, this);
			action->setProperty ("TabIndex", i);
			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (navigateToTab ()));
			connect (action,
					SIGNAL (triggered ()),
					widget,
					SLOT (deleteLater ()));

			QToolButton *button = new QToolButton ();
			button->setDefaultAction (action);
			button->setToolButtonStyle (Qt::ToolButtonTextBesideIcon);
			button->setSizePolicy (QSizePolicy::Expanding,
					button->sizePolicy ().verticalPolicy ());
			button->setProperty ("OrigText", origText);
			layout->addWidget (button);

			if (currentIdx == i)
				toFocus = button;
		}

		widget->setLayout (layout);
		layout->update ();
		layout->activate ();

		const QRect& rect = QApplication::desktop ()->
				screenGeometry (Proxy_->GetMainWindow ());
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
		Proxy_->GetTabWidget ()->setCurrentIndex (idx);
	}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_tabslist, LeechCraft::TabsList::Plugin);
