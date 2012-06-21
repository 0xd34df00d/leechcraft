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

#include "trayview.h"
#include <QSortFilterProxyModel>
#include <QDeclarativeContext>
#include <interfaces/iremovabledevmanager.h>

namespace LeechCraft
{
namespace Vrooby
{
	TrayView::TrayView (QWidget *parent)
	: QDeclarativeView (parent)
	, Proxy_ (new QSortFilterProxyModel (this))
	{
		setWindowFlags (Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		setAttribute (Qt::WA_TranslucentBackground);

		setResizeMode (SizeRootObjectToView);
		setFixedSize (250, 300);

		rootContext ()->setContextProperty ("devModel", Proxy_);
		setSource (QUrl ("qrc:/vrooby/resources/qml/DevicesTrayView.qml"));
	}

	void TrayView::SetDevModel (QAbstractItemModel *model)
	{
		Proxy_->setSourceModel (model);
	}
}
}
