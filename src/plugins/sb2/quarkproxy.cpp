/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkproxy.h"
#include <QQuickItem>
#include <QToolTip>
#include <QApplication>
#include <QToolBar>
#include <QQmlEngine>
#include <QMainWindow>
#include <QtDebug>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/geometry.h>
#include <util/gui/autoresizemixin.h>
#include <util/util.h>
#include "viewmanager.h"
#include "sbview.h"
#include "quarkunhidelistview.h"
#include "quarkorderview.h"
#include "declarativewindow.h"
#include "viewsettingsmanager.h"
#include "quarkmanager.h"
#include "panelsettingsdialog.h"

namespace LC::SB2
{
	QuarkProxy::QuarkProxy (ViewManager *mgr, QObject *parent)
	: QObject (parent)
	, Manager_ (mgr)
	{
	}

	const QString& QuarkProxy::GetExtHoveredQuarkClass () const
	{
		return ExtHoveredQuarkClass_;
	}

	QRect QuarkProxy::GetFreeCoords () const
	{
		return Manager_->GetFreeCoords ();
	}

	QPoint QuarkProxy::mapToGlobal (double x, double y)
	{
		return Manager_->GetView ()->mapToGlobal (QPoint (x, y));
	}

	void QuarkProxy::showTextTooltip (int x, int y, const QString& str)
	{
		QToolTip::showText ({ x, y }, str);
	}

	void QuarkProxy::removeQuark (const QUrl& url)
	{
		Manager_->RemoveQuark (url);
	}

	QRect QuarkProxy::getWinRect ()
	{
		return GetFreeCoords ();
	}

	QPoint QuarkProxy::fitRect (const QPoint& src, const QSize& size, const QRect& rect, bool canOverlap)
	{
		Util::FitFlags flags = Util::FitFlag::NoFlags;
		if (!canOverlap)
			flags |= Util::FitFlag::NoOverlap;

		return rect.isValid () ?
				Util::FitRect (src, size, rect, flags) :
				Util::FitRectScreen (src, size, flags);
	}

	void QuarkProxy::registerAutoresize (const QPoint& src, const QVariant& var)
	{
		const auto win = var.value<QWindow*> ();
		if (!win)
		{
			qWarning () << Q_FUNC_INFO
					<< "no window";
			return;
		}
		new Util::AutoResizeMixin (src, [this] { return Manager_->GetFreeCoords (); }, win);
	}

	void QuarkProxy::panelMoveRequested (const QString& position)
	{
		Qt::ToolBarArea area = Qt::BottomToolBarArea;

		if (position == QLatin1String ("left"))
			area = Qt::LeftToolBarArea;
		else if (position == QLatin1String ("right"))
			area = Qt::RightToolBarArea;
		else if (position == QLatin1String ("top"))
			area = Qt::TopToolBarArea;
		else if (position == QLatin1String ("bottom"))
			area = Qt::BottomToolBarArea;
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown position"
					<< position;

		Manager_->MovePanel (area);
	}

	void QuarkProxy::quarkAddRequested (int x, int y)
	{
		auto toAdd = Manager_->FindAllQuarks ();
		for (const auto& existing : Manager_->GetAddedQuarks ())
		{
			const auto pos = std::find_if (toAdd.begin (), toAdd.end (),
					[&existing] (const auto& item) { return item->Url_ == existing; });
			if (pos == toAdd.end ())
				continue;

			toAdd.erase (pos);
		}

		if (toAdd.isEmpty ())
			return;

		auto unhide = new QuarkUnhideListView (toAdd, Manager_, Manager_->GetView ());
		new Util::AutoResizeMixin ({ x, y }, [this] { return Manager_->GetFreeCoords (); }, unhide);
		unhide->show ();
	}

	void QuarkProxy::quarkOrderRequested (int x, int y)
	{
		if (QuarkOrderView_)
		{
			QuarkOrderView_->deleteLater ();
			return;
		}

		QuarkOrderView_ = new QuarkOrderView (Manager_);

		const auto& pos = Util::FitRect ({ x, y }, QuarkOrderView_->size (), GetFreeCoords (),
				Util::FitFlag::NoOverlap);
		QuarkOrderView_->move (pos);
		QuarkOrderView_->show ();

		connect (QuarkOrderView_,
				&QuarkOrderView::quarkClassHovered,
				this,
				[this] (const QString& qClass)
				{
					if (ExtHoveredQuarkClass_ == qClass)
						return;

					ExtHoveredQuarkClass_ = qClass;
					emit extHoveredQuarkClassChanged ();
				});
	}

	void QuarkProxy::panelSettingsRequested ()
	{
		QList<SettingsItem> xsds
		{
			{
				tr ("SB2 panel settings"),
				GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon (),
				Manager_->GetViewSettingsManager ()->GetXSD ()
			}
		};

		for (const auto& added : Manager_->GetAddedQuarks ())
		{
			const auto& addedManager = Manager_->GetAddedQuarkManager (added);
			if (!addedManager->HasSettings ())
				continue;

			const auto& manifest = addedManager->GetManifest ();
			xsds.append ({
					manifest.GetName (),
					manifest.GetIcon (),
					addedManager->GetXSD ()
				});
		}

		PanelSettingsDialog dia { xsds };
		dia.exec ();
	}

	QString QuarkProxy::prettySize (qint64 size)
	{
		return Util::MakePrettySize (size);
	}

	QString QuarkProxy::prettySizeShort (qint64 size)
	{
		return Util::MakePrettySizeShort (size);
	}

	QString QuarkProxy::prettyTime (qint64 time)
	{
		return Util::MakeTimeFromLong (time);
	}

	void QuarkProxy::instantiateQuark (const QUrl& url, QObject *obj)
	{
		Manager_->SetupContext (url, QQmlEngine::contextForObject (obj));
		obj->setProperty ("active", true);
	}
}
