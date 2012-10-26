/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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
#include <QDeclarativeContext>
#include <QtDebug>
#include <util/util.h>
#include <util/sys/paths.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoretabwidget.h>

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
				TabName
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

	TabListView::TabListView (const QList<QWidget*>& widgets, ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, Proxy_ (proxy)
	, Model_ (new TabsListModel (this))
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

		auto ictw = proxy->GetTabWidget ();
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
			item->setData (ictw->TabText (idx), TabsListModel::Roles::TabName);

			const auto& px = ictw->TabIcon (idx).pixmap (32, 32);
			item->setData (Util::GetAsBase64Src (px.toImage ()), TabsListModel::Roles::TabIcon);

			Model_->appendRow (item);
		}

		rootContext ()->setContextProperty ("tabsListModel", Model_);
		setSource (QUrl::fromLocalFile (file));
	}
}
}
