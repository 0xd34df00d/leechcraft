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

#include "devicesbrowserwidget.h"
#include <QMessageBox>
#include <QInputDialog>
#include <util/models/flattenfiltermodel.h>
#include <util/util.h>
#include <interfaces/iremovabledevmanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/lmp/isyncplugin.h>
#include "core.h"
#include "devicesuploadmodel.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class MountableFlattener : public Util::FlattenFilterModel
		{
		public:
			MountableFlattener (QObject *parent)
			: Util::FlattenFilterModel (parent)
			{
			}

			QVariant data (const QModelIndex& index, int role) const
			{
				if (role != Qt::DisplayRole)
					return Util::FlattenFilterModel::data (index, role);

				const auto& mounts = index.data (DeviceRoles::MountPoints).toStringList ();
				const auto& mountText = mounts.isEmpty () ?
						tr ("not mounted") :
						tr ("mounted at %1").arg (mounts.join ("; "));

				const auto& size = index.data (DeviceRoles::TotalSize).toLongLong ();
				return QString ("%1 (%2, %3), %4")
						.arg (index.data (DeviceRoles::VisibleName).toString ())
						.arg (Util::MakePrettySize (size))
						.arg (index.data (DeviceRoles::DevFile).toString ())
						.arg (mountText);
			}
		protected:
			bool IsIndexAccepted (const QModelIndex& child) const
			{
				return child.data (DeviceRoles::IsMountable).toBool ();
			}
		};
	}

	DevicesBrowserWidget::DevicesBrowserWidget (QWidget *parent)
	: QWidget (parent)
	, DevMgr_ (0)
	, CurrentSyncer_ (0)
	{
		Ui_.setupUi (this);
		Ui_.UploadButton_->setIcon (Core::Instance ().GetProxy ()->GetIcon ("svn-commit"));

		auto model = new DevicesUploadModel (this);
		model->setSourceModel (Core::Instance ().GetLocalCollection ()->GetCollectionModel ());
		Ui_.OurCollection_->setModel (model);
	}

	void DevicesBrowserWidget::InitializeDevices ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		const auto& mgrs = pm->GetAllCastableTo<IRemovableDevManager*> ();
		if (mgrs.isEmpty ())
		{
			setEnabled (false);
			return;
		}

		DevMgr_ = mgrs.at (0);

		auto flattener = new MountableFlattener (this);
		flattener->SetSource (DevMgr_->GetDevicesModel ());
		Ui_.DevicesSelector_->setModel (flattener);

		Ui_.DevicesSelector_->setCurrentIndex (-1);

		connect (flattener,
				SIGNAL (dataChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleDevDataChanged (QModelIndex, QModelIndex)));
	}

	void DevicesBrowserWidget::handleDevDataChanged (const QModelIndex& from, const QModelIndex& to)
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx >= from.row () && idx <= to.row ())
			on_DevicesSelector__activated (idx);
	}

	void DevicesBrowserWidget::on_UploadButton__released ()
	{

	}

	void DevicesBrowserWidget::on_DevicesSelector__activated (int idx)
	{
		CurrentSyncer_ = 0;

		if (idx < 0)
		{
			Ui_.MountButton_->setEnabled (false);
			return;
		}

		auto isMounted = Ui_.DevicesSelector_->itemData (idx, DeviceRoles::IsMounted).toBool ();
		Ui_.MountButton_->setEnabled (!isMounted);

		if (!isMounted)
			return;

		const auto& mountPath = Ui_.DevicesSelector_->itemData (idx, DeviceRoles::MountPoints).toStringList ().value (0);
		if (mountPath.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "device seems to be mounted, but no mount points available:"
					<< Ui_.DevicesSelector_->itemData (idx, DeviceRoles::DevID).toString ();
			return;
		}

		QList<ISyncPlugin*> suitables;

		auto syncers = Core::Instance ().GetSyncPlugins ();
		Q_FOREACH (auto syncer, syncers)
		{
			auto isp = qobject_cast<ISyncPlugin*> (syncer);
			if (isp->CouldSync (mountPath) != SyncConfLevel::None)
				suitables << isp;
		}

		if (suitables.isEmpty ())
		{
			QMessageBox::warning (this,
					"LeechCraft",
					tr ("No plugins are able to sync this device."));
			return;
		}

		if (suitables.size () == 1)
			CurrentSyncer_ = suitables.value (0);
		else
		{
			QStringList items;
			Q_FOREACH (ISyncPlugin *plugin, suitables)
				items << plugin->GetSyncSystemName ();

			const auto& name = QInputDialog::getItem (this,
					tr ("Select syncer"),
					tr ("Multiple different syncers can handle the device %1, what do you want to use?")
						.arg (Ui_.DevicesSelector_->itemText (idx)),
					items);
			if (name.isEmpty ())
				return;

			CurrentSyncer_ = suitables.value (items.indexOf (name));
		}

		if (!CurrentSyncer_)
			return;

		auto lay = Ui_.UpOptsTab_->layout ();
		while (lay->count ())
			lay->removeItem (lay->itemAt (0));

		if (QWidget *w = CurrentSyncer_->MakeSyncParamsWidget ())
			lay->addWidget (w);
	}

	void DevicesBrowserWidget::on_MountButton__released ()
	{
		const int idx = Ui_.DevicesSelector_->currentIndex ();
		if (idx < 0)
			return;

		const auto& id = Ui_.DevicesSelector_->itemData (idx, DeviceRoles::DevID).toString ();
		DevMgr_->MountDevice (id);
	}
}
}
