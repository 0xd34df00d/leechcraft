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

#include "tablistview.h"
#include <QStandardItemModel>
#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QtDebug>
#include <util/util.h>
#include <util/sys/paths.h>
#include <util/qml/colorthemeproxy.h>
#include <util/gui/unhoverdeletemixin.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoretabwidget.h>
#include "themeimageprovider.h"

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		class TabsListModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				TabIcon = Qt::UserRole + 1,
				TabName,
				TabWidgetObj
			};

			TabsListModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> names;
				names [Roles::TabIcon] = "tabIcon";
				names [Roles::TabName] = "tabName";
				setRoleNames (names);
			}
		};
	}

	TabListView::TabListView (const QByteArray& tc,
			const QList<QWidget*>& widgets, ICoreTabWidget *ictw, ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, Proxy_ (proxy)
	, ICTW_ (ictw)
	, TC_ (tc)
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

		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

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

		rootContext ()->setContextProperty ("tabsListModel", Model_);
		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		rootContext ()->setContextProperty ("longestText", longestText);
		engine ()->addImageProvider ("ThemeIcons", new ThemeImageProvider (proxy));
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
}
