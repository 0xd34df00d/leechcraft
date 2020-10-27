/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tablistview.h"
#include <QStandardItemModel>
#include <QMainWindow>
#include <QQmlContext>
#include <QQuickItem>
#include <util/util.h>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/util.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/models/rolenamesmixin.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoretabwidget.h>

namespace LC::SB2
{
	namespace
	{
		class TabsListModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				TabIcon = Qt::UserRole + 1,
				TabName,
				TabWidgetObj
			};

			explicit TabsListModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> names;
				names [Roles::TabIcon] = "tabIcon";
				names [Roles::TabName] = "tabName";
				setRoleNames (names);
			}
		};
	}

	TabListView::TabListView (QByteArray tc, const QList<QWidget*>& widgets,
			ICoreTabWidget *ictw, QMainWindow *win, ICoreProxy_ptr proxy, QWidget *parent)
	: QQuickWidget (parent)
	, Proxy_ (proxy)
	, ICTW_ (ictw)
	, MW_ (win)
	, TC_ (std::move (tc))
	, Model_ (new TabsListModel (this))
	, UnhoverDeleteMixin_ (new Util::UnhoverDeleteMixin (this))
	{
		const auto& file = Util::GetSysPath (Util::SysPath::QML, "sb2", "TabListView.qml");
		if (file.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "file not found";
			deleteLater ();
			return;
		}

		setWindowFlags (Qt::ToolTip);
		Util::EnableTransparency (this);

		QString longestText;

		for (auto w : widgets)
		{
			const int idx = ictw->IndexOf (w);
			if (idx < 0)
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown widget"
						<< w;
				continue;
			}

			auto item = new QStandardItem;

			const auto& tabText = ictw->TabText (idx);
			item->setData (tabText, TabsListModel::Roles::TabName);
			if (tabText.size () > longestText.size ())
				longestText = tabText;

			const auto& px = ictw->TabIcon (idx).pixmap (32, 32);
			item->setData (Util::GetAsBase64Src (px.toImage ()), TabsListModel::Roles::TabIcon);

			item->setData (QVariant::fromValue<QObject*> (w), TabsListModel::Roles::TabWidgetObj);

			Model_->appendRow (item);

			auto itw = qobject_cast<ITabWidget*> (w);
			auto parent	= itw->ParentMultiTabs ();
			connect (parent,
					SIGNAL (removeTab (QWidget*)),
					this,
					SLOT (handleTabRemoved (QWidget*)));
		}

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("tabsListModel", Model_);
		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		rootContext ()->setContextProperty ("longestText", longestText);
		engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
		setSource (QUrl::fromLocalFile (file));

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
		connect (rootObject (),
				SIGNAL (tabSwitchRequested (int)),
				this,
				SLOT (switchToItem (int)));
		connect (rootObject (),
				SIGNAL (tabCloseRequested (int)),
				this,
				SLOT (closeItem (int)));
	}

	TabListView::~TabListView ()
	{
		Model_->clear ();
	}

	QByteArray TabListView::GetTabClass () const
	{
		return TC_;
	}

	void TabListView::HandleLauncherHovered ()
	{
		UnhoverDeleteMixin_->Stop ();
	}

	void TabListView::HandleLauncherUnhovered ()
	{
		UnhoverDeleteMixin_->Start (1200);
	}

	void TabListView::handleTabRemoved (QWidget *widget)
	{
		for (int i = 0; i < Model_->rowCount (); ++i)
		{
			auto item = Model_->item (i);
			auto widgetObj = item->data (TabsListModel::Roles::TabWidgetObj).value<QObject*> ();
			if (widgetObj != widget)
				continue;

			Model_->removeRow (i);
			return;
		}
	}

	void TabListView::switchToItem (int idx)
	{
		auto item = Model_->item (idx);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "null item at"
					<< idx;
			return;
		}

		auto widgetObj = item->data (TabsListModel::Roles::TabWidgetObj).value<QObject*> ();
		ICTW_->setCurrentWidget (static_cast<QWidget*> (widgetObj));
		MW_->activateWindow ();
		MW_->raise ();
		deleteLater ();
	}

	void TabListView::closeItem (int idx)
	{
		auto item = Model_->item (idx);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "null item at"
					<< idx;
			return;
		}

		auto widgetObj = item->data (TabsListModel::Roles::TabWidgetObj).value<QObject*> ();
		auto itw = qobject_cast<ITabWidget*> (widgetObj);
		itw->Remove ();
	}
}
