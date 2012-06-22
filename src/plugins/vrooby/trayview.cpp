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
#include <QIcon>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeImageProvider>
#include <QGraphicsObject>
#include "flatmountableitems.h"
#include "devbackend.h"

namespace LeechCraft
{
namespace Vrooby
{
	namespace
	{
		class MountIconProvider : public QDeclarativeImageProvider
		{
			ICoreProxy_ptr Proxy_;
		public:
			MountIconProvider (ICoreProxy_ptr proxy)
			: QDeclarativeImageProvider (Pixmap)
			, Proxy_ (proxy)
			{
			}

			QPixmap requestPixmap (const QString& id, QSize *size, const QSize& requestedSize)
			{
				const auto& icon = Proxy_->GetIcon (id);
				if (size)
					*size = icon.actualSize (requestedSize);
				return icon.pixmap (requestedSize);
			}
		};
	}

	TrayView::TrayView (ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (parent)
	, CoreProxy_ (proxy)
	, Flattened_ (new FlatMountableItems (this))
	, Backend_ (0)
	{
		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		setAttribute (Qt::WA_TranslucentBackground);
		setAttribute (Qt::WA_OpaquePaintEvent, false);

		setResizeMode (SizeRootObjectToView);
		setFixedSize (500, 250);

		engine ()->addImageProvider ("mountIcons", new MountIconProvider (proxy));

		rootContext ()->setContextProperty ("devModel", Flattened_);
		setSource (QUrl ("qrc:/vrooby/resources/qml/DevicesTrayView.qml"));
	}

	void TrayView::SetBackend (DevBackend *backend)
	{
		if (Backend_)
			disconnect (rootObject (),
					SIGNAL (toggleMountRequested (const QString&)),
					Backend_,
					SLOT (toggleMount (QString)));

		Backend_ = backend;
		connect (rootObject (),
				SIGNAL (toggleMountRequested (const QString&)),
				Backend_,
				SLOT (toggleMount (QString)));

		Flattened_->SetSource (Backend_->GetDevicesModel ());
	}
}
}
