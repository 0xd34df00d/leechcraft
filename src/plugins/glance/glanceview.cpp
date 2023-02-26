/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "glanceview.h"
#include <QStandardItemModel>
#include <QQuickPaintedItem>
#include <QQuickWidget>
#include <QQmlEngine>
#include <QQmlContext>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/ihavetabs.h>
#include <util/qml/util.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/colorthemeproxy.h>
#include <util/models/rolenamesmixin.h>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>

namespace LC::Glance
{
	namespace
	{
		enum ModelRoles
		{
			ThumbId = Qt::UserRole + 1,
		};

		class ThumbsModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			explicit ThumbsModel (QObject *parent)
			: RoleNamesMixin { parent }
			{
				setRoleNames ({ { ModelRoles::ThumbId, "thumbId" } });
			}
		};
	}

	class ThumbsProvider : public QQuickImageProvider
	{
		QVector<QPixmap> Pixmaps_;
	public:
		explicit ThumbsProvider ()
		: QQuickImageProvider { Pixmap }
		{
		}

		void AddPixmap (const QPixmap& px)
		{
			Pixmaps_ << px;
		}

		void RemovePixmap (int idx)
		{
			Pixmaps_.removeAt (idx);
		}

		QPixmap requestPixmap (const QString& idStr, QSize *size, const QSize&) override
		{
			const auto& px = Pixmaps_.value (idStr.toInt ());
			*size = px.size ();
			return px;
		}
	};

	GlanceView::GlanceView (ICoreTabWidget& ictw, QObject *parent)
	: QObject { parent }
	, Tabs_ { ictw }
	, View_ { std::make_unique<QQuickWidget> () }
	, ThumbsModel_ { *new ThumbsModel { this } }
	, ThumbsProvider_ { *new ThumbsProvider }
	{
		Util::SetupFullscreenView (*View_);
		Util::EnableTransparency (*View_);
		Util::WatchQmlErrors (*View_);

		auto& engine = *View_->engine ();
		engine.addImageProvider ("theme"_qs, new Util::ThemeImageProvider { GetProxyHolder () });
		engine.addImageProvider ("thumbs"_qs, &ThumbsProvider_);

		auto& rootCtx = *View_->rootContext ();
		rootCtx.setContextProperty ("view"_qs, this);
		rootCtx.setContextProperty ("thumbsModel"_qs, &ThumbsModel_);
		rootCtx.setContextProperty ("colorProxy"_qs,
				new Util::ColorThemeProxy { GetProxyHolder ()->GetColorThemeManager (), this });

		View_->setSource (Util::GetSysPathUrl (Util::SysPath::QML, "glance"_qs, "View.qml"_qs));

		QTimer::singleShot (0, this, &GlanceView::Start);
	}

	GlanceView::~GlanceView () = default;

	void GlanceView::finish ()
	{
		deleteLater ();
	}

	void GlanceView::selectItem (int idx)
	{
		Tabs_.setCurrentTab (idx);
		deleteLater ();
	}

	void GlanceView::deleteItem (int idx)
	{
		if (idx < 0 || idx >= Tabs_.WidgetCount ())
		{
			qCritical () << "GlanceView: index outside of bounds"
					<< idx;
			return;
		}

		qobject_cast<ITabWidget*> (Tabs_.Widget (idx))->Remove ();
		ThumbsProvider_.RemovePixmap (idx);
		ThumbsModel_.removeRow (idx);

		if (Tabs_.WidgetCount () < 2)
			deleteLater ();
	}

	namespace
	{
		auto GetTabSize (ICoreTabWidget& ictw)
		{
			return ictw.Widget (ictw.CurrentIndex ())->size ();
		}
	}

	void GlanceView::Start ()
	{
		const auto count = Tabs_.WidgetCount ();
		if (count < 2)
		{
			deleteLater ();
			return;
		}

		const auto tabSize = GetTabSize (Tabs_);

		for (int i = 0; i < count; ++i)
		{
			const auto w = Tabs_.Widget (i);
			QPixmap pixmap { tabSize };
			w->render (&pixmap);
			ThumbsProvider_.AddPixmap (pixmap);

			const auto item = new QStandardItem;
			item->setData (i, ModelRoles::ThumbId);
			ThumbsModel_.appendRow (item);
		}

		View_->show ();
		View_->setFocus ();
	}
}
