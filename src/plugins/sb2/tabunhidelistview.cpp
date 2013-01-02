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

#include "tabunhidelistview.h"
#include <QStandardItemModel>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/util.h>
#include <util/qml/colorthemeproxy.h>
#include <util/gui/unhoverdeletemixin.h>
#include "themeimageprovider.h"
#include "unhidelistmodel.h"

namespace LeechCraft
{
namespace SB2
{
	TabUnhideListView::TabUnhideListView (const QList<TabClassInfo>& tcs, ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, Model_ (new UnhideListModel (this))
	{
		new Util::UnhoverDeleteMixin (this);

		const auto& file = Util::GetSysPath (Util::SysPath::QML, "sb2", "UnhideListView.qml");
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

		rootContext ()->setContextProperty ("unhideListModel", Model_);
		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		engine ()->addImageProvider ("ThemeIcons", new ThemeImageProvider (proxy));
		setSource (QUrl::fromLocalFile (file));

		for (const auto& tc : tcs)
		{
			auto item = new QStandardItem;
			item->setData (tc.TabClass_, UnhideListModel::Roles::ItemClass);
			item->setData (tc.VisibleName_, UnhideListModel::Roles::ItemName);
			item->setData (tc.Description_, UnhideListModel::Roles::ItemDescription);
			item->setData (Util::GetAsBase64Src (tc.Icon_.pixmap (32, 32).toImage ()),
					UnhideListModel::Roles::ItemIcon);
			Model_->appendRow (item);
		}

		connect (rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
		connect (rootObject (),
				SIGNAL (itemUnhideRequested (QString)),
				this,
				SLOT (unhide (QString)),
				Qt::QueuedConnection);
	}

	void TabUnhideListView::unhide (const QString& idStr)
	{
		const auto& id = idStr.toUtf8 ();
		emit unhideRequested (id);

		for (int i = 0; i < Model_->rowCount (); ++i)
			if (Model_->item (i)->data (UnhideListModel::Roles::ItemClass).toByteArray () == id)
			{
				Model_->removeRow (i);
				break;
			}

		if (!Model_->rowCount ())
			deleteLater ();
	}
}
}
