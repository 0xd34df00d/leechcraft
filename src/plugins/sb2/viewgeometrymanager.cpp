/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewgeometrymanager.h"
#include <QToolBar>
#include <QApplication>
#include <QQmlContext>
#include <QMainWindow>
#include <QtDebug>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <util/sll/scopeguards.h>

#ifdef WITH_X11
#include <util/x11/xwrapper.h>
#endif

#include "viewmanager.h"
#include "sbview.h"
#include "viewsettingsmanager.h"

namespace LC::SB2
{
	ViewGeometryManager::ViewGeometryManager (ViewManager *parent)
	: QObject (parent)
	, ViewMgr_ (parent)
	{
		const auto& xsm = ViewMgr_->GetViewSettingsManager ()->GetXSM ();
		xsm->RegisterObject ("PanelSize", this,
				[this] (const QVariant& prop)
				{
					ViewMgr_->GetView ()->SetDimensions (prop.toInt ());
				});
	}

	void ViewGeometryManager::Manage ()
	{
		const auto pos = [this]
		{
			auto settings = ViewMgr_->GetSettings ();
			auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("Toolbars"));
			const auto& posSettingName = "Pos_" + QString::number (ViewMgr_->GetWindowIndex ());
			return settings->value (posSettingName, static_cast<int> (Qt::LeftToolBarArea)).toInt ();
		} ();

		auto toolbar = ViewMgr_->GetToolbar ();
#ifdef WITH_X11
		if (!ViewMgr_->IsDesktopMode ())
		{
#endif
			toolbar->setFloatable (false);
			toolbar->setMovable (false);
#ifdef WITH_X11
		}
		else
		{
			toolbar->setParent (nullptr);
			toolbar->setWindowFlags (Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

			toolbar->setFloatable (true);
			toolbar->setAllowedAreas (Qt::NoToolBarArea);

			toolbar->setAttribute (Qt::WA_X11NetWmWindowTypeDock);
			toolbar->setAttribute (Qt::WA_X11NetWmWindowTypeToolBar, false);
			toolbar->setAttribute (Qt::WA_AlwaysShowToolTips);

			toolbar->show ();

			Util::XWrapper::Instance ().MoveWindowToDesktop (toolbar->winId (), 0xffffffff);
		}
#endif

		SetPosition (static_cast<Qt::ToolBarArea> (pos));
	}

	namespace
	{
		QRect ToGeom (const QRect& screenGeometry, const QSize& minSize, Qt::ToolBarArea area, QSize *diff = nullptr)
		{
			const int addition = 2;
			switch (area)
			{
			case Qt::BottomToolBarArea:
			{
				QRect rect { 0, 0, screenGeometry.width (), minSize.height () + addition };
				rect.moveLeft (screenGeometry.left ());
				rect.moveBottom (screenGeometry.bottom ());

				if (diff)
					*diff = { 0, addition };

				return rect;
			}
			case Qt::TopToolBarArea:
			{
				QRect rect { 0, 0, screenGeometry.width (), minSize.height () + addition };
				rect.moveLeft (screenGeometry.left ());
				rect.moveTop (screenGeometry.top ());

				if (diff)
					*diff = { 0, addition };

				return rect;
			}
			case Qt::LeftToolBarArea:
			{
				QRect rect { 0, 0, minSize.width () + addition, screenGeometry.height () };
				rect.moveLeft (screenGeometry.left ());
				rect.moveTop (screenGeometry.top ());

				if (diff)
					*diff = { addition, 0 };

				return rect;
			}
			case Qt::RightToolBarArea:
			{
				QRect rect { 0, 0, minSize.width () + addition, screenGeometry.height () };
				rect.moveRight (screenGeometry.right ());
				rect.moveTop (screenGeometry.top ());

				if (diff)
					*diff = { addition, 0 };

				return rect;
			}
			default:
				qWarning () << Q_FUNC_INFO
						<< "unsupported area"
						<< area;
				return { { 0, 0 }, minSize };
			}
		}
	}

	void ViewGeometryManager::SetPosition (Qt::ToolBarArea pos)
	{
		SetOrientation ((pos == Qt::LeftToolBarArea || pos == Qt::RightToolBarArea) ? Qt::Vertical : Qt::Horizontal);

		auto toolbar = ViewMgr_->GetToolbar ();

		{
			auto settings = ViewMgr_->GetSettings ();
			const auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("Toolbars"));
			settings->setValue ("Pos_" + QString::number (ViewMgr_->GetWindowIndex ()), static_cast<int> (pos));
		}

#ifdef WITH_X11
		if (!ViewMgr_->IsDesktopMode ())
		{
#endif
			ViewMgr_->GetManagedWindow ()->addToolBar (static_cast<Qt::ToolBarArea> (pos), toolbar);
#ifdef Q_OS_MAC
			// dunno WTF
			ViewMgr_->GetManagedWindow ()->show ();
#endif
#ifdef WITH_X11
		}
		else
		{
			auto& xwrapper = Util::XWrapper::Instance ();

			xwrapper.ClearStrut (toolbar);
			xwrapper.Sync ();

			const auto screenGeometry = xwrapper.GetAvailableGeometry (ViewMgr_->GetManagedWindow ());

			QSize diff;
			const auto& rect = ToGeom (screenGeometry, ViewMgr_->GetView ()->minimumSizeHint (), pos, &diff);

			toolbar->setGeometry (rect);
			ViewMgr_->GetView ()->setFixedSize (rect.size () - diff);
			toolbar->setFixedSize (rect.size ());

			xwrapper.SetStrut (toolbar, pos);
		}
#endif
	}

	void ViewGeometryManager::SetOrientation (Qt::Orientation orientation)
	{
		auto view = ViewMgr_->GetView ();
		const auto& size = view->sizeHint ();

		switch (orientation)
		{
		case Qt::Vertical:
			view->resize (size);
			view->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);
			view->rootContext ()->setContextProperty (QStringLiteral ("viewOrient"), "vertical");
			break;
		case Qt::Horizontal:
			view->resize (size);
			view->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
			view->rootContext ()->setContextProperty (QStringLiteral ("viewOrient"), "horizontal");
			break;
		}
	}
}
