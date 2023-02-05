/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagerwindow.h"
#include <QUrl>
#include <QStandardItemModel>
#include <QScreen>
#include <QApplication>
#include <QSysInfo>
#include <QQmlContext>
#include <QQmlEngine>
#include <QtDebug>
#include <util/sys/paths.h>
#include <util/gui/autoresizemixin.h>
#include <util/gui/unhoverdeletemixin.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/settableiconprovider.h>
#include <util/qml/util.h>
#include <util/models/rolenamesmixin.h>
#include <util/x11/xwrapper.h>
#include <xcb/xcb.h>
#include <xcb/render.h>
#include <xcb/composite.h>
#include <xcb/xcb_renderutil.h>
#include "pagerwindowproxy.h"

namespace LC
{
namespace Krigstask
{
	class DesktopsModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
	public:
		enum Role
		{
			SubModel = Qt::UserRole + 1,
			DesktopName,
			DesktopID,
			IsCurrent
		};

		DesktopsModel (QObject *parent)
		: RoleNamesMixin<QStandardItemModel> (parent)
		{
			QHash<int, QByteArray> roleNames;
			roleNames [Role::SubModel] = "subModel";
			roleNames [Role::DesktopName] = "desktopName";
			roleNames [Role::DesktopID] = "desktopID";
			roleNames [Role::IsCurrent] = "isCurrent";
			setRoleNames (roleNames);
		}
	};

	class SingleDesktopModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
	public:
		enum Role
		{
			WinName = Qt::UserRole + 1,
			WID,
			IsActive
		};

		SingleDesktopModel (QObject *parent)
		: RoleNamesMixin<QStandardItemModel> (parent)
		{
			QHash<int, QByteArray> roleNames;
			roleNames [Role::WinName] = "winName";
			roleNames [Role::WID] = "wid";
			roleNames [Role::IsActive] = "isActive";
			setRoleNames (roleNames);
		}
	};

	class ImageProvider : public QQuickImageProvider
	{
		QHash<QString, QPixmap> Images_;
	public:
		ImageProvider ()
		: QQuickImageProvider (Pixmap)
		{
		}

		void SetImage (const QString& id, const QPixmap& px)
		{
			Images_ [id] = px;
		}

		QPixmap requestPixmap (const QString& id, QSize *size, const QSize&)
		{
			const auto& img = Images_.value (id);
			if (img.isNull ())
				return {};

			if (size)
				*size = img.size ();

			return img;
		}
	};

	PagerWindow::PagerWindow (const QScreen *screen, bool showThumbs, ICoreProxy_ptr proxy, QWidget *parent)
	: QQuickWidget (parent)
	, DesktopsModel_ (new DesktopsModel (this))
	, ShowThumbs_ (showThumbs)
	, WinIconProv_ (new Util::SettableIconProvider)
	, WinSnapshotProv_ (new ImageProvider)
	{
		new Util::UnhoverDeleteMixin (this);

		setWindowFlags (Qt::ToolTip);
		Util::EnableTransparency (*this);

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));

		engine ()->addImageProvider ("WinIcons", WinIconProv_);
		engine ()->addImageProvider ("WinSnaps", WinSnapshotProv_);

		rootContext ()->setContextProperty ("geometry", screen->availableGeometry ());

		FillModel ();
		rootContext ()->setContextProperty ("showThumbs", ShowThumbs_);
		rootContext ()->setContextProperty ("desktopsModel", DesktopsModel_);

		const auto pagerProxy = new PagerWindowProxy { this };
		connect (pagerProxy,
				SIGNAL (showDesktop (int)),
				this,
				SLOT (showDesktop (int)));
		connect (pagerProxy,
				SIGNAL (showWindow (qulonglong)),
				this,
				SLOT (showWindow (qulonglong)));
		rootContext ()->setContextProperty ("pagerProxy", pagerProxy);

		setResizeMode (SizeViewToRootObject);

		setSource (Util::GetSysPathUrl (Util::SysPath::QML, "krigstask", "Pager.qml"));
	}

	void PagerWindow::FillModel ()
	{
		auto& w = Util::XWrapper::Instance ();

		const auto numDesktops = w.GetDesktopCount ();
		if (numDesktops <= 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown desktop count";
			deleteLater ();
			return;
		}

		QMap<int, QList<ulong>> desk2wins;
		for (auto wid : w.GetWindows ())
			if (w.ShouldShow (wid))
				desk2wins [w.GetWindowDesktop (wid)] << wid;

		const auto curDesk = w.GetCurrentDesktop ();
		const auto activeApp = w.GetActiveApp ();

		const auto& deskNames = w.GetDesktopNames ();

		for (int i = 0; i < numDesktops; ++i)
		{
			auto subModel = new SingleDesktopModel (this);
			FillSubmodel (subModel, desk2wins [i], activeApp);

			auto item = new QStandardItem;
			item->setData (QVariant::fromValue<QObject*> (subModel), DesktopsModel::Role::SubModel);

			const auto& deskName = deskNames.value (i, "Desktop " + QString::number (i + 1));
			item->setData (deskName, DesktopsModel::Role::DesktopName);

			item->setData (curDesk == i, DesktopsModel::Role::IsCurrent);
			item->setData (i, DesktopsModel::Role::DesktopID);
			DesktopsModel_->appendRow (item);
		}
	}

	namespace
	{
		const xcb_format_t* GetFormat (uint8_t depth)
		{
			const auto setup = xcb_get_setup (QX11Info::connection ());
			for (auto i = xcb_setup_pixmap_formats_iterator (setup); i.rem; xcb_format_next (&i))
					if (i.data->depth == depth)
						return i.data;

			return {};
		}

		QImage::Format GetImageFormat (uint8_t depth, const xcb_visualtype_t *visual)
		{
			const auto format = GetFormat (depth);

			if (!visual || !format)
				return QImage::Format_Invalid;

			if (depth == 32 &&
					format->bits_per_pixel == 32 &&
					visual->red_mask == 0xff0000 &&
					visual->green_mask == 0xff00 &&
					visual->blue_mask == 0xff)
				return QImage::Format_ARGB32_Premultiplied;

			if (depth == 24 &&
					format->bits_per_pixel == 32 &&
					visual->red_mask == 0xff0000 &&
					visual->green_mask == 0xff00 &&
					visual->blue_mask == 0xff)
				return QImage::Format_RGB32;

			if (depth == 16 &&
					format->bits_per_pixel == 16 &&
					visual->red_mask == 0xf800 &&
					visual->green_mask == 0x7e0 &&
					visual->blue_mask == 0x1f)
				return QImage::Format_RGB16;

			return QImage::Format_Invalid;
		}

		const xcb_visualtype_t* GetVisual (const xcb_visualid_t visualId)
		{
			const auto sit = xcb_setup_roots_iterator (xcb_get_setup (QX11Info::connection ()));

			for (auto dit = xcb_screen_allowed_depths_iterator (sit.data);
					dit.rem; xcb_depth_next (&dit))
				 for (auto vit = xcb_depth_visuals_iterator (dit.data);
						vit.rem; xcb_visualtype_next (&vit))
					  if (vit.data->visual_id == visualId)
						  return vit.data;

			return {};
		}

		QImage FromXcb (xcb_pixmap_t pixmap,
				int width, int height, int depth, xcb_visualid_t visualId)
		{
			const auto conn = QX11Info::connection ();
			const auto getCookie = xcb_get_image_unchecked (conn,
					XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap,
					0, 0, width, height, 0xffffffff);

			const std::shared_ptr<xcb_get_image_reply_t> imageReply
			{
				xcb_get_image_reply (conn, getCookie, nullptr),
				&free
			};

			if (!imageReply)
				return {};

			auto data = xcb_get_image_data (imageReply.get ());
			const auto length = xcb_get_image_data_length (imageReply.get ());

			const auto format = GetImageFormat (depth, GetVisual (visualId));
			if (format == QImage::Format_Invalid)
				return {};

			const auto bpl = length / height;

			QImage image { data, width, height, bpl, format };
			const auto byteOrder = xcb_get_setup (conn)->image_byte_order;

			if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && byteOrder == XCB_IMAGE_ORDER_MSB_FIRST) ||
				(QSysInfo::ByteOrder == QSysInfo::BigEndian && byteOrder == XCB_IMAGE_ORDER_LSB_FIRST))
			{
				for (int i = 0; i < height; i++)
					switch (format)
					{
					case QImage::Format_RGB16:
					{
						auto px = reinterpret_cast<ushort*> (image.scanLine (i));
						const auto end = px + width;
						for (; px != end; ++px)
							*px = ((*px << 8) & 0xff00) | ((*px >> 8) & 0x00ff);
						break;
					}
					case QImage::Format_RGB32:
					case QImage::Format_ARGB32_Premultiplied:
					{
						auto px = reinterpret_cast<QRgb*> (image.scanLine (i));
						const auto end = px + width;
						for (; px != end; ++px)
							*px = ((*px << 24) & 0xff000000) | ((*px << 8) & 0x00ff0000) |
									((*px >> 8) & 0x0000ff00) | ((*px >> 24) & 0x000000ff);
						break;
					}
					default:
						return {};
					}
			}

			if (format == QImage::Format_RGB32)
			{
				auto bits = reinterpret_cast<QRgb*> (image.bits ());
				for (int y = 0; y < height; ++y)
				{
					for (int x = 0; x < width; ++x)
						bits [x] |= 0xff000000;
					bits += bpl / 4;
				}
			}

			return image.copy ();
		}

		QImage GrabWindow (ulong wid)
		{
			const auto conn = QX11Info::connection ();

			const auto sizeCookie = xcb_get_geometry (conn, wid);
			const std::shared_ptr<xcb_get_geometry_reply_t> sizeReply
				{ xcb_get_geometry_reply (conn, sizeCookie, nullptr), &free };

			const auto attrsCookie = xcb_get_window_attributes (conn, wid);
			const std::shared_ptr<xcb_get_window_attributes_reply_t> attrsReply
				{ xcb_get_window_attributes_reply (conn, attrsCookie, nullptr), &free };

			xcb_composite_redirect_window (conn, wid, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

			const auto backing = xcb_generate_id (conn);
			xcb_composite_name_window_pixmap (conn, wid, backing);

			const auto formatsCookie = xcb_render_query_pict_formats_unchecked (conn);
			const std::shared_ptr<xcb_render_query_pict_formats_reply_t> formats
			{
				xcb_render_query_pict_formats_reply (conn, formatsCookie, nullptr),
				&free
			};

			const auto format = xcb_render_util_find_visual_format (formats.get (), attrsReply->visual);
			if (!format)
				return {};

			const auto picture = xcb_generate_id (conn);
			const uint32_t params [] = { XCB_SUBWINDOW_MODE_INCLUDE_INFERIORS };
			xcb_render_create_picture (conn,
					picture,
					backing,
					format->format,
					XCB_RENDER_CP_SUBWINDOW_MODE,
					params);

			const auto region = xcb_generate_id (conn);
			xcb_xfixes_create_region_from_window (conn, region, wid, XCB_SHAPE_SK_BOUNDING);
			xcb_xfixes_translate_region (conn, region, sizeReply->x, sizeReply->y);
			xcb_xfixes_set_picture_clip_region (conn, picture, region, 0, 0);
			xcb_xfixes_destroy_region (conn, region);

			const auto xpixmap = xcb_generate_id (conn);
			xcb_create_pixmap (conn, sizeReply->depth, xpixmap,
					Util::XWrapper::Instance ().GetRootWindow (),
					sizeReply->width, sizeReply->height);

			xcb_render_composite (conn, XCB_RENDER_PICT_OP_OVER, picture, 0, xpixmap,
					0, 0, 0, 0,
					0, 0, sizeReply->width, sizeReply->height);

			const auto& image = FromXcb (xpixmap,
					sizeReply->width, sizeReply->height, sizeReply->depth, attrsReply->visual);

			xcb_free_pixmap (conn, xpixmap);
			xcb_render_free_picture (conn, picture);
			xcb_free_pixmap (conn, backing);
			xcb_composite_unredirect_window (conn, wid, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

			return image;
		}
/*
		void CreatePicture (xcb_pixmap_t pixmap, int depth)
		{
			if (pixmap == XCB_PIXMAP_NONE)
				return;

			static QHash<int, xcb_render_pictformat_t> renderFormats;
			if (renderFormats.contains (depth))
				return;

			const auto conn = QX11Info::connection ();
			const auto cookie = xcb_render_query_pict_formats_unchecked (conn);
			const std::shared_ptr<xcb_render_query_pict_formats_reply_t> formats
			{
				xcb_render_query_pict_formats_reply (conn, cookie, nullptr),
				&free
			};

			if (!formats)
				return;

			for (auto it = xcb_render_query_pict_formats_formats_iterator (formats.get ());
					it.rem; xcb_render_pictforminfo_next (&it))
				if (it.data->depth == depth)
				{
					renderFormats [depth] = it.data->id;
					break;
				}
		}

		QImage GrabWindow (ulong wid)
		{
			const auto conn = QX11Info::connection ();

			xcb_composite_redirect_window (conn, wid, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

			const auto sizeCookie = xcb_get_geometry (conn, wid);
			const std::shared_ptr<xcb_get_geometry_reply_t> sizeReply
				{ xcb_get_geometry_reply (conn, sizeCookie, nullptr), &free };

			const auto xpixmap = xcb_generate_id (conn);
			xcb_create_pixmap (conn, 32, xpixmap, wid, sizeReply->width, sizeReply->height);

			CreatePicture (xpixmap, 32);

			xcb_render_composite (conn, XCB_RENDER_PICT_OP_SRC, nullptr, XCB_RENDER_PICTURE_NONE,

			xcb_composite_unredirect_window (conn, wid, XCB_COMPOSITE_REDIRECT_AUTOMATIC);

			return {};
		}
		*/
	}

	void PagerWindow::FillSubmodel (SingleDesktopModel *model,
			const QList<ulong>& windows, ulong active)
	{
		auto& w = Util::XWrapper::Instance ();

		for (auto wid : windows)
		{
			const auto& widStr = QString::number (wid);
			WinIconProv_->SetIcon ({ widStr }, w.GetWindowIcon (wid));
			WinSnapshotProv_->SetImage (widStr, QPixmap::fromImage (GrabWindow (wid)));

			auto item = new QStandardItem;
			item->setData (w.GetWindowTitle (wid), SingleDesktopModel::Role::WinName);
			item->setData (static_cast<qulonglong> (wid), SingleDesktopModel::Role::WID);
			item->setData (wid == active, SingleDesktopModel::Role::IsActive);
			model->appendRow (item);
		}
	}

	void PagerWindow::showDesktop (int id)
	{
		Util::XWrapper::Instance ().SetCurrentDesktop (id);
		for (auto i = 0; i < DesktopsModel_->rowCount (); ++i)
			DesktopsModel_->item (i)->setData (i == id, DesktopsModel::Role::IsCurrent);
	}

	void PagerWindow::showWindow (qulonglong wid)
	{
		auto& w = Util::XWrapper::Instance ();
		const auto desk = w.GetWindowDesktop (wid);
		showDesktop (desk);
		w.RaiseWindow (wid);

		for (int i = 0, count = w.GetDesktopCount (); i < count; ++i)
		{
			auto deskItem = DesktopsModel_->item (i);
			const auto& modelVar = deskItem->data (DesktopsModel::Role::SubModel);

			auto model = dynamic_cast<SingleDesktopModel*> (modelVar.value<QObject*> ());
			for (int j = 0; j < model->rowCount (); ++j)
			{
				auto winItem = model->item (j);
				const auto thatID = winItem->data (SingleDesktopModel::Role::WID).toULongLong ();
				winItem->setData (thatID == wid, SingleDesktopModel::Role::IsActive);
			}
		}
	}
}
}
