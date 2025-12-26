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
#include <interfaces/devices/deviceroles.h>
#include <util/sll/containerconversions.h>
#include <util/models/modeliterator.h>
#include <util/sll/qtutil.h>
#include <util/sys/paths.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/util.h>
#include <util/util.h>
#include "devbackend.h"

namespace LC
{
namespace Vrooby
{
	class TrayModel : public QSortFilterProxyModel
	{
		QSet<QString> Hidden_;
		bool FilterEnabled_ = true;
	public:
		enum CustomRoles
		{
			FormattedTotalSize = MassStorageRole::MassStorageRoleMax + 1,
			FormattedFreeSpace,
			UsedPercentage,
			MountButtonIcon,
			ToggleHiddenIcon,
			MountedAt
		};

		explicit TrayModel (QObject *parent)
		: QSortFilterProxyModel { parent }
		{
			QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Vrooby");
			settings.beginGroup ("HiddenDevices");
			Hidden_ = Util::AsSet (settings.value ("List").toStringList ());
			settings.endGroup ();
		}

		QVariant data (const QModelIndex& index, int role) const override
		{
			switch (role)
			{
			case CustomRoles::ToggleHiddenIcon:
			{
				const auto& id = index.data (CommonDevRole::DevPersistentID).toString ();
				return Hidden_.contains (id) ?
						"image://ThemeIcons/list-add"_qs :
						"image://ThemeIcons/list-remove"_qs;
			}
			case CustomRoles::FormattedTotalSize:
			{
				const auto size = index.data (MassStorageRole::TotalSize).toLongLong ();
				return tr ("total size: %1").arg (Util::MakePrettySize (size));
			}
			case CustomRoles::FormattedFreeSpace:
			{
				const auto size = index.data (MassStorageRole::AvailableSize).toLongLong ();
				return tr ("available size: %1").arg (Util::MakePrettySize (size));
			}
			case CustomRoles::MountButtonIcon:
				return index.data (MassStorageRole::IsMounted).toBool () ?
						"image://ThemeIcons/emblem-unmounted"_qs :
						"image://ThemeIcons/emblem-mounted"_qs;
			case CustomRoles::MountedAt:
			{
				const auto& mounts = index.data (MassStorageRole::MountPoints).toStringList ();
				return mounts.isEmpty () ?
						QString {} :
						tr ("Mounted at %1").arg (mounts.join ("; "));
			}
			case CustomRoles::UsedPercentage:
			{
				const qint64 free = index.data (MassStorageRole::AvailableSize).value<qint64> ();
				if (free < 0)
					return -1;

				const double total = index.data (MassStorageRole::TotalSize).value<qint64> ();
				return (1 - free / total) * 100;
			}
			default:
				return QSortFilterProxyModel::data (index, role);
			}
		}

		QHash<int, QByteArray> roleNames () const override
		{
			static const QHash<int, QByteArray> names
			{
				{ MassStorageRole::VisibleName, "devName"_qba },
				{ MassStorageRole::DevFile, "devFile"_qba },
				{ MassStorageRole::IsRemovable, "isRemovable"_qba },
				{ MassStorageRole::IsPartition, "isPartition"_qba },
				{ MassStorageRole::IsMountable, "isMountable"_qba },
				{ CommonDevRole::DevID, "devID"_qba },
				{ CommonDevRole::DevPersistentID, "devPersistentID"_qba },
				{ MassStorageRole::AvailableSize, "availableSize"_qba },
				{ MassStorageRole::TotalSize, "totalSize"_qba },
				{ CustomRoles::FormattedTotalSize, "formattedTotalSize"_qba },
				{ CustomRoles::FormattedFreeSpace, "formattedFreeSpace"_qba },
				{ CustomRoles::UsedPercentage, "usedPercentage"_qba },
				{ CustomRoles::MountButtonIcon, "mountButtonIcon"_qba },
				{ CustomRoles::ToggleHiddenIcon, "toggleHiddenIcon"_qba },
				{ CustomRoles::MountedAt, "mountedAt"_qba },
			};
			return names;
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
		bool filterAcceptsRow (int row, const QModelIndex&) const override
		{
			const auto& idx = sourceModel ()->index (row, 0);
			if (!idx.data (MassStorageRole::IsMountable).toBool ())
				return false;

			if (!FilterEnabled_)
				return true;

			const auto& id = idx.data (CommonDevRole::DevPersistentID).toString ();
			return !Hidden_.contains (id);
		}
	};

	TrayView::TrayView (ICoreProxy_ptr proxy)
	: CoreProxy_ (proxy)
	, TrayModel_ (new TrayModel (this))
	{
		setWindowFlags (Qt::ToolTip);
		Util::EnableTransparency (*this);

		setResizeMode (SizeRootObjectToView);
		setFixedSize (500, 250);

		engine ()->addImageProvider ("ThemeIcons"_qs, new Util::ThemeImageProvider (proxy));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			engine ()->addImportPath (cand);

		rootContext ()->setContextProperty ("colorProxy"_qs,
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), this));
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
		bool prevHasItems = HasItems ();;
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
}
