/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trayview.h"
#include <QSortFilterProxyModel>
#include <QIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QQuickItem>
#include <QSortFilterProxyModel>
#include <QSettings>
#include <QCoreApplication>
#include <util/sll/containerconversions.h>
#include <util/sys/paths.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/util.h>
#include "flatmountableitems.h"
#include "devbackend.h"

namespace LC
{
namespace Vrooby
{
	class FilterModel : public QSortFilterProxyModel
	{
		bool FilterEnabled_;
		QSet<QString> Hidden_;
	public:
		FilterModel (QObject *parent)
		: QSortFilterProxyModel (parent)
		, FilterEnabled_ (true)
		{
			setDynamicSortFilter (true);

			QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Vrooby");
			settings.beginGroup ("HiddenDevices");
			Hidden_ = Util::AsSet (settings.value ("List").toStringList ());
			settings.endGroup ();
		}

		QVariant data (const QModelIndex& index, int role) const
		{
			if (role != FlatMountableItems::ToggleHiddenIcon)
				return QSortFilterProxyModel::data (index, role);

			const auto& id = index.data (CommonDevRole::DevPersistentID).toString ();
			return Hidden_.contains (id) ?
					"image://ThemeIcons/list-add" :
					"image://ThemeIcons/list-remove";
		}

		void ToggleHidden (const QString& id)
		{
			if (!Hidden_.remove (id))
				Hidden_ << id;

			QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Vrooby");
			settings.beginGroup ("HiddenDevices");
			settings.setValue ("List", QStringList (Hidden_.values ()));
			settings.endGroup ();

			if (FilterEnabled_)
				invalidateFilter ();
			else
			{
				for (int i = 0; i < rowCount (); ++i)
				{
					const auto& idx = sourceModel ()->index (i, 0);
					if (id != idx.data (CommonDevRole::DevPersistentID).toString ())
						continue;

					const auto& mapped = mapFromSource (idx);
					emit dataChanged (mapped, mapped);
				}
			}

			if (Hidden_.isEmpty ())
			{
				FilterEnabled_ = true;
				invalidateFilter ();
			}
		}

		int GetHiddenCount () const
		{
			return Hidden_.size ();
		}

		void ToggleFilter ()
		{
			FilterEnabled_ = !FilterEnabled_;
			invalidateFilter ();
		}
	protected:
		bool filterAcceptsRow (int row, const QModelIndex&) const
		{
			if (!FilterEnabled_)
				return true;

			const auto& idx = sourceModel ()->index (row, 0);
			const auto& id = idx.data (CommonDevRole::DevPersistentID).toString ();
			return !Hidden_.contains (id);
		}
	};

	TrayView::TrayView (ICoreProxy_ptr proxy)
	: CoreProxy_ (proxy)
	, Flattened_ (new FlatMountableItems (this))
	, Filtered_ (new FilterModel (this))
	, Backend_ (0)
	{
		Filtered_->setSourceModel (Flattened_);

		setWindowFlags (Qt::ToolTip);
		Util::EnableTransparency (*this);

		setResizeMode (SizeRootObjectToView);
		setFixedSize (500, 250);

		engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (proxy));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
		rootContext ()->setContextProperty ("devModel", Filtered_);
		rootContext ()->setContextProperty ("devicesLabelText", tr ("Removable devices"));
		rootContext ()->setContextProperty ("hasHiddenItems", Filtered_->GetHiddenCount ());
		setSource (Util::GetSysPathUrl (Util::SysPath::QML, "vrooby", "DevicesTrayView.qml"));

		connect (Flattened_,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SIGNAL (hasItemsChanged ()));
		connect (Flattened_,
				SIGNAL (rowsRemoved (QModelIndex, int, int)),
				this,
				SIGNAL (hasItemsChanged ()));

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
		if (Backend_)
			disconnect (rootObject (),
					0,
					Backend_,
					0);

		Backend_ = backend;
		connect (rootObject (),
				SIGNAL (toggleMountRequested (QString)),
				Backend_,
				SLOT (toggleMount (QString)));

		Flattened_->SetSource (Backend_->GetDevicesModel ());
	}

	bool TrayView::HasItems () const
	{
		return Flattened_->rowCount ();
	}

	void TrayView::toggleHide (const QString& persId)
	{
		Filtered_->ToggleHidden (persId);

		rootContext ()->setContextProperty ("hasHiddenItems", Filtered_->GetHiddenCount ());
	}

	void TrayView::toggleShowHidden ()
	{
		Filtered_->ToggleFilter ();
	}
}
}
