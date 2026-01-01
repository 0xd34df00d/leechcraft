/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trayview.h"
#include <QIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QQuickItem>
#include <interfaces/devices/deviceroles.h>
#include <util/models/modeliterator.h>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/util.h>
#include <util/util.h>
#include "devbackend.h"
#include "trayproxymodel.h"

namespace LC::Vrooby
{
	TrayView::TrayView ()
	: TrayModel_ (new TrayProxyModel (this))
	{
		setWindowFlags (Qt::ToolTip);
		Util::EnableTransparency (*this);

		setResizeMode (SizeRootObjectToView);
		setFixedSize (500, 250);

		engine ()->addImageProvider ("ThemeIcons"_qs, new Util::ThemeImageProvider (GetProxyHolder ()));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("colorProxy"_qs,
				new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this));
		rootContext ()->setContextProperty ("devModel"_qs, TrayModel_);
		rootContext ()->setContextProperty ("devicesLabelText"_qs, tr ("Removable devices"));
		rootContext ()->setContextProperty ("hasHiddenItems"_qs, TrayModel_->GetHiddenCount ());
		setSource (Util::GetSysPathUrl (Util::SysPath::QML, "vrooby"_qs, "DevicesTrayView.qml"_qs));

		connect (rootObject (),
				SIGNAL (toggleHideRequested (QString)),
				this,
				SLOT (toggleHide (QString)));
		connect (rootObject (),
				SIGNAL (toggleShowHidden ()),
				this,
				SLOT (toggleShowHidden ()));
	}

	void TrayView::SetBackend (DevBackend *backend)
	{
		const bool prevHasItems = HasItems ();
		if (Backend_)
		{
			disconnect (rootObject (),
					nullptr,
					Backend_,
					nullptr);
			disconnect (Backend_->GetDevicesModel (),
					nullptr,
					this,
					nullptr);
		}

		Backend_ = backend;
		connect (rootObject (),
				SIGNAL (toggleMountRequested (QString)),
				Backend_,
				SLOT (toggleMount (QString)));

		const auto model = Backend_->GetDevicesModel ();
		TrayModel_->setSourceModel (model);
		connect (model,
				&QAbstractItemModel::rowsInserted,
				this,
				&TrayView::hasItemsChanged);
		connect (model,
				&QAbstractItemModel::rowsRemoved,
				this,
				&TrayView::hasItemsChanged);

		if (prevHasItems != HasItems ())
			emit hasItemsChanged ();
	}

	bool TrayView::HasItems () const
	{
		if (!Backend_)
			return false;

		return std::ranges::any_of (Util::AllModelRows (*Backend_->GetDevicesModel ()),
				[] (const QModelIndex& idx)
				{
					return idx.data (MassStorageRole::IsMountable).toBool ();
				});
	}

	void TrayView::toggleHide (const QString& persId)
	{
		TrayModel_->ToggleHidden (persId);

		rootContext ()->setContextProperty ("hasHiddenItems"_qs, TrayModel_->GetHiddenCount ());
	}

	void TrayView::toggleShowHidden ()
	{
		TrayModel_->ToggleFilter ();
	}
}
