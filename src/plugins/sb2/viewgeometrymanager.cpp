/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "viewgeometrymanager.h"
#include <QSettings>
#include <QToolBar>
#include <QApplication>
#include <QQmlContext>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QtDebug>
#include <xmlsettingsdialog/basesettingsmanager.h>

#ifdef WITH_X11
#include <util/x11/xwrapper.h>
#endif

#include "viewmanager.h"
#include "sbview.h"
#include "viewsettingsmanager.h"

namespace LC
{
namespace SB2
{
	ViewGeometryManager::ViewGeometryManager (ViewManager *parent)
	: QObject (parent)
	, ViewMgr_ (parent)
	{
		const auto& xsm = ViewMgr_->GetViewSettingsManager ()->GetXSM ();
		xsm->RegisterObject ("PanelSize", this, "updatePanelSize");
		updatePanelSize ();
	}

	void ViewGeometryManager::Manage ()
	{
		auto settings = ViewMgr_->GetSettings ();
		settings->beginGroup ("Toolbars");
		const auto& posSettingName = "Pos_" + QString::number (ViewMgr_->GetWindowIndex ());
		auto pos = settings->value (posSettingName, static_cast<int> (Qt::LeftToolBarArea)).toInt ();
		settings->endGroup ();

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
		QRect ToGeom (const QRect& screenGeometry, const QSize& minSize, Qt::ToolBarArea area, QSize *diff = 0)
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
		setOrientation ((pos == Qt::LeftToolBarArea || pos == Qt::RightToolBarArea) ? Qt::Vertical : Qt::Horizontal);

		auto toolbar = ViewMgr_->GetToolbar ();

		auto settings = ViewMgr_->GetSettings ();
		settings->beginGroup ("Toolbars");
		settings->setValue ("Pos_" + QString::number (ViewMgr_->GetWindowIndex ()), static_cast<int> (pos));
		settings->endGroup ();

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

	void ViewGeometryManager::setOrientation (Qt::Orientation orientation)
	{
		auto view = ViewMgr_->GetView ();
		const auto& size = view->sizeHint ();

		switch (orientation)
		{
		case Qt::Vertical:
			view->resize (size);
			view->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);
			view->rootContext ()->setContextProperty ("viewOrient", "vertical");
			break;
		case Qt::Horizontal:
			view->resize (size);
			view->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Preferred);
			view->rootContext ()->setContextProperty ("viewOrient", "horizontal");
			break;
		}
	}

	void ViewGeometryManager::updatePanelSize ()
	{
		const auto& xsm = ViewMgr_->GetViewSettingsManager ()->GetXSM ();
		ViewMgr_->GetView ()->SetDimensions (xsm->property ("PanelSize").toInt ());
	}
}
}
