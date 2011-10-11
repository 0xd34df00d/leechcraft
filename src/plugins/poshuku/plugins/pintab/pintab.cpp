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

#include "pintab.h"
#include <QAbstractButton>
#include <QCoreApplication>
#include <QDebug>
#include <QIcon>
#include <QList>
#include <QMenu>
#include <QMetaObject>
#include <QSettings>
#include <QTabWidget>
#include <QUrl>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>

namespace LeechCraft
{
namespace Poshuku
{
namespace PinTab
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		CoreProxy_ = proxy;

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Poshuku");
		int size = settings.beginReadArray ("Pinned tabs");

		for (int i = 0; i < size; i++)
		{
			settings.setArrayIndex (i);
			PinnedUrls_.append (settings.value ("url").toString ());
		}
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
		SavePinned();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.PinTab";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku pintab";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Pinned tabs support for the Poshuku browser.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Poshuku.Plugins/1.0";
		return result;
	}

	void Plugin::hookMoreMenuFillEnd (LeechCraft::IHookProxy_ptr proxy,
			QMenu *menu, QGraphicsWebView *webView, QObject *browserWidget)
	{
		QAction *pintab = new QAction (tr ("Pin tab"), browserWidget);

		connect (pintab,
				SIGNAL (triggered ()),
				this,
				SLOT (handlePinTabTriggered ()));
		ActionsMap_ [browserWidget] = pintab;
	}

	void Plugin::handlePinTabTriggered ()
	{
		QAction *pintab = qobject_cast<QAction*> (sender ());

		if (!pintab)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< sender ()
					<< "to QAction";
			return;
		}

		SetPinned (pintab->parent (), pintab, !Pinned_.contains (pintab->parent ()));
	}

	void Plugin::hookTabBarContextMenuActions (LeechCraft::IHookProxy_ptr proxy,
		const QObject *browserWidget) const
	{
		QList<QObject*> acts;

		acts.append (ActionsMap_ [browserWidget]);
		proxy->SetValue ("endActions" , QVariant::fromValue (acts));
	}

	void Plugin::hookTabRemoveRequested (LeechCraft::IHookProxy_ptr proxy, QObject *browserWidget)
	{
		if (Pinned_.contains (browserWidget))
			proxy->CancelDefault ();
	}

	void Plugin::handlePinnedTitleChanged (const QString& title)
	{
		if (title.isEmpty ())
			return;

		QObject *browserwidget = sender ();

		Pinned_ [browserwidget] = title;
		ChangeTabTitle (browserwidget, "");
	}

	void Plugin::ChangeTabTitle (QObject *widget, const QString& title)
	{
		QObject *poshuku = CoreProxy_->GetPluginsManager ()->
					GetPluginByID ("org.LeechCraft.Poshuku");
		QWidget *browserWidget = qobject_cast<QWidget*> (widget);

		if (!browserWidget)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable cast"
					<< widget
					<< "to QWidget";
			return;
		}

		QMetaObject::invokeMethod (poshuku,
			"changeTabName",
			Qt::QueuedConnection,
			Q_ARG (QWidget*, browserWidget),
			Q_ARG (const QString&, title));
	}

	void Plugin::SetPinned (QObject *widget, QAction *action, bool pinned )
	{
		const QString& title = pinned ? "" : Pinned_.value (widget);
		const QString& actionText = pinned ? tr ("Unpin tab") : tr ("Pin tab");
		QGraphicsWebView *webView = GetWebView (widget);

		if (!webView)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to get QGraphicsWebView from"
					<< widget;
			return;
		}

		const QString& tabTitle = webView->title ();

		if (pinned)
		{
			Pinned_[widget] = tabTitle;
			connect (widget,
					SIGNAL (titleChanged (const QString&)),
					this,
					SLOT (handlePinnedTitleChanged (const QString&)));
		}
		else
		{
			Pinned_.remove (widget);
			disconnect (widget,
					SIGNAL (titleChanged (const QString&)),
					this,
					SLOT (handlePinnedTitleChanged (const QString&)));
		}
		action->setText (actionText);
		ChangeTabTitle (widget, title);
		SavePinned ();
	}

	void Plugin::hookTabAdded (LeechCraft::IHookProxy_ptr,
		QObject *browserWidget, QGraphicsWebView *view, const QUrl& url)
	{
		if (PinnedUrls_.size () && PinnedUrls_.contains (url.toString ()))
		{
			SetPinned (browserWidget, ActionsMap_ [browserWidget], true);
			PinnedUrls_.removeOne (url.toString ());
		}
	}

	void Plugin::SavePinned()
	{
		QStringList pinned;

		Q_FOREACH (QObject *browserWidget, Pinned_.keys ())
		{
			QGraphicsWebView *webView = GetWebView (browserWidget);

			if (!webView)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to get QGraphicsWebView from"
						<< browserWidget;
				continue;
			}

			pinned.append (webView->url ().toString ());
		}


		QSettings settings (QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Poshuku");

		settings.remove ("Pinned tabs");
		settings.beginWriteArray ("Pinned tabs");
		for (int i = 0; i < pinned.size (); i++)
		{
			settings.setArrayIndex (i);
			settings.setValue ("url", pinned [i]);
		}
		settings.endArray();
	}

	QGraphicsWebView* Plugin::GetWebView (QObject *browserWidget)
	{
		QGraphicsWebView *view = NULL;

		QMetaObject::invokeMethod (browserWidget,
			"getWebView",
			Q_RETURN_ARG (QGraphicsWebView*, view));

		return view;
	}


}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_poshuku_pintab, LeechCraft::Poshuku::PinTab::Plugin);
