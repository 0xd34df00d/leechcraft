/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/ihavetabs.h>

Q_DECLARE_METATYPE (ICoreTabWidget*)

namespace LC
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
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QList<QAction*> Plugin::GetActions (ActionsEmbedPlace aep) const
	{
		QList<QAction*> actions;
		if (aep == ActionsEmbedPlace::QuickLaunch)
			actions << ShowList_;
		return actions;
	}

	QMap<QByteArray, ActionInfo> Plugin::GetActionInfo () const
	{
		const auto& iconName = ShowList_->property ("ActionIcon").toString ();
		ActionInfo info
		{
			ShowList_->text (),
			ShowList_->shortcut (),
			Proxy_->GetIconThemeManager ()->GetIcon (iconName),
		};
		return { { "ShowList", info } };
	}

	void Plugin::SetShortcut (const QByteArray&, const QKeySequences_t& seqs)
	{
		ShowList_->setShortcuts (seqs);
	}

	void Plugin::RemoveTab (ICoreTabWidget *ictw, int idx)
	{
		const auto tab = qobject_cast<ITabWidget*> (ictw->Widget (idx));
		tab->Remove ();

		handleShowList ();
	}

	namespace
	{
		class ListEventFilter : public QObject
		{
			QList<QToolButton*> AllButtons_;
			QString SearchText_;

			QTimer NumSelectTimer_;

			Plugin * const Plugin_;
			QWidget * const Widget_;
		public:
			ListEventFilter (const QList<QToolButton*>& buttons, Plugin *plugin, QWidget *parent)
			: QObject (parent)
			, AllButtons_ (buttons)
			, Plugin_ (plugin)
			, Widget_ (parent)
			{
				NumSelectTimer_.setSingleShot (true);
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
					for (auto button : AllButtons_)
						if (button->hasFocus ())
							button->animateClick ();
					return true;
				case Qt::Key_Up:
					AllButtons_.last ()->setFocus ();
					return true;
				case Qt::Key_Down:
					AllButtons_.first ()->setFocus ();
					return true;
				case Qt::Key_PageUp:
					PerformWithFocusButton ([this] (int idx) { MoveUp (idx, 5); });
					break;
				case Qt::Key_PageDown:
					PerformWithFocusButton ([this] (int idx) { MoveDown (idx, 5); });
					break;
				case Qt::Key_Home:
					AllButtons_.first ()->setFocus ();
					break;
				case Qt::Key_End:
					AllButtons_.last ()->setFocus ();
					break;
				case Qt::Key_Delete:
					PerformWithFocusButton ([this] (int index) -> void
						{
							const auto button = AllButtons_ [index];
							const auto ictw = button->defaultAction ()->
									property ("ICTW").value<ICoreTabWidget*> ();
							Widget_->deleteLater ();
							Plugin_->RemoveTab (ictw, index);
						});
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
			void MoveUp (int idx, int count) const
			{
				AllButtons_ [std::max (0, idx - count)]->setFocus ();
			}

			void MoveDown (int idx, int count) const
			{
				const auto last = AllButtons_.size () - 1;
				AllButtons_ [std::min (idx + count, last)]->setFocus ();
			}

			template<typename T>
			void PerformWithFocusButton (T action) const
			{
				for (int i = 0; i < AllButtons_.size (); ++i)
					if (AllButtons_ [i]->hasFocus ())
					{
						action (i);
						return;
					}
			}

			void FocusSearch ()
			{
				bool isNum = false;
				const auto srcNum = SearchText_.toInt (&isNum);
				const auto num = srcNum - 1;
				if (isNum && srcNum >= 0 && srcNum <= AllButtons_.size ())
				{
					if (!srcNum && !AllButtons_.isEmpty ())
						AllButtons_.last ()->animateClick ();
					else if (srcNum * 10 - 1 >= AllButtons_.size ())
						AllButtons_ [num]->animateClick ();
					else
					{
						if (NumSelectTimer_.isActive ())
						{
							NumSelectTimer_.stop ();
							disconnect (&NumSelectTimer_,
									0,
									0,
									0);
						}

						NumSelectTimer_.start (QApplication::keyboardInputInterval ());
						connect (&NumSelectTimer_,
								SIGNAL (timeout ()),
								AllButtons_ [num],
								SLOT (animateClick ()));

						AllButtons_ [num]->setFocus ();
					}

					return;
				}

				for (auto butt : AllButtons_)
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

		const auto scroll = new QScrollArea { nullptr };
		scroll->setWindowFlags (Qt::Popup | Qt::FramelessWindowHint);
		scroll->setAttribute (Qt::WA_TranslucentBackground);
		scroll->setWindowModality (Qt::ApplicationModal);

		const auto widget = new QWidget { nullptr };

		QVBoxLayout *layout = new QVBoxLayout ();
		layout->setSpacing (1);
		layout->setContentsMargins (0, 0, 0, 0);

		const int currentIdx = tw->CurrentIndex ();
		QToolButton *toFocus = nullptr;
		QList<QToolButton*> allButtons;
		for (int i = 0, count = tw->WidgetCount (); i < count; ++i)
		{
			const QString& origText = tw->TabText (i);
			QString title = QString ("[%1] ").arg (i + 1) + origText;
			if (title.size () > 100)
				title = title.left (100) + "...";
			QAction *action = new QAction (tw->TabIcon (i), title, this);
			action->setToolTip (origText);
			action->setProperty ("TabIndex", i);
			action->setProperty ("ICTW", QVariant::fromValue<ICoreTabWidget*> (tw));
			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (navigateToTab ()));
			connect (action,
					SIGNAL (triggered ()),
					scroll,
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

		scroll->installEventFilter (new ListEventFilter (allButtons, this, scroll));
		widget->setLayout (layout);
		layout->update ();
		layout->activate ();

		const QRect& rect = QApplication::desktop ()->
				screenGeometry (rootWM->GetPreferredWindow ());
		auto pos = rect.center ();

		const auto maxHeight = rect.height () * 2 / 3;

		const auto& size = widget->sizeHint ();
		const auto trueHeight = std::min (size.height (), maxHeight);
		const bool isScrolling = size.height () > trueHeight;
		pos -= QPoint (size.width () / 2, trueHeight / 2);

		widget->setMinimumSize (size.width (), trueHeight);

		scroll->setMaximumHeight (maxHeight);
		scroll->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
		scroll->setMinimumHeight (trueHeight + 6);

		scroll->setWidget (widget);

		const auto width = scroll->sizeHint ().width () +
				(isScrolling ? scroll->verticalScrollBar ()->width () : 0);
		scroll->resize (width, trueHeight + 6);

		scroll->move (pos);
		scroll->show ();

		if (toFocus)
			toFocus->setFocus ();
	}

	void Plugin::navigateToTab ()
	{
		const int idx = sender ()->property ("TabIndex").toInt ();
		const auto ictw = sender ()->property ("ICTW").value<ICoreTabWidget*> ();
		ictw->setCurrentTab (idx);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_tabslist, LC::TabsList::Plugin);
