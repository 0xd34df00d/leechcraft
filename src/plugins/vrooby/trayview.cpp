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

			QPixmap requestPixmap (const QString& id, QSize *size, const QSize&)
			{
				const auto& icon = Proxy_->GetIcon (id);
				if (size)
					*size = icon.actualSize (QSize (32, 32));
				return icon.pixmap (QSize (32, 32));
			}
		};
	}

	TrayView::TrayView (ICoreProxy_ptr proxy, QWidget *parent)
	: QDeclarativeView (0)
	, CoreProxy_ (proxy)
	, Flattened_ (new FlatMountableItems (this))
	, Backend_ (0)
	{
		setStyleSheet ("background: transparent");
		setWindowFlags (Qt::ToolTip);
		setAttribute (Qt::WA_TranslucentBackground);

		setResizeMode (SizeRootObjectToView);
		setFixedSize (500, 250);

		engine ()->addImageProvider ("mountIcons", new MountIconProvider (proxy));

		rootContext ()->setContextProperty ("devModel", Flattened_);
		rootContext ()->setContextProperty ("devicesLabelText", tr ("Removable devices"));
		setSource (QUrl ("qrc:/vrooby/resources/qml/DevicesTrayView.qml"));

		connect (Flattened_,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SIGNAL (hasItemsChanged ()));
		connect (Flattened_,
				SIGNAL (rowsRemoved (QModelIndex, int, int)),
				this,
				SIGNAL (hasItemsChanged ()));
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

	bool TrayView::HasItems () const
	{
		return Flattened_->rowCount ();
	}
}
}
