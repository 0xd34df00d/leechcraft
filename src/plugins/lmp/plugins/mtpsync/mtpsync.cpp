/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mtpsync.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/devices/iremovabledevmanager.h>
#include <interfaces/devices/deviceroles.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilmputilproxy.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>

namespace LC::LMP::MTPSync
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		DevicesModel_ = new Util::SimpleRoledItemsModel<ModelRow> { this };

		Mtp_ = std::make_unique<MtpRunner> ();
		connect (&*Mtp_,
				&MtpRunner::mtpConnected,
				this,
				&Plugin::AddDevices);
		connect (&*Mtp_,
				&MtpRunner::mtpDisconnected,
				this,
				[this] (const QList<MtpDeviceInfo>& devices)
				{
					const auto& items = DevicesModel_->GetItems ();
					for (const auto& dev : devices)
					{
						const auto pos = std::ranges::find_if (items,
								[&dev] (const ModelRow& row) { return row.Serial_ == dev.Serial_; });
						if (pos != items.end ())
							DevicesModel_->RemoveItem (pos - items.begin ());
					}
				});

		[] (auto *mtp, Plugin *plugin) -> Util::ContextTask<void>
		{
			co_await Util::AddContextObject { *plugin };
			plugin->AddDevices (co_await mtp->Run (&Mtp::GetCurrentDevices));
		} (&*Mtp_, this);

		RefreshSyncTargets ();
	}

	namespace
	{
		QList<UsbDevice> GetModelDevices (const QAbstractItemModel& model,
				const QModelIndex& parent, int from, int to)
		{
			if (parent.isValid ())
				return {};

			QList<UsbDevice> devices;
			for (int i = from; i <= to; ++i)
			{
				const auto& idx = model.index (i, 0);
				devices << UsbDevice { idx.data (Busnum).toInt (), idx.data (Devnum).toInt () };
			}
			return devices;
		}
	}

	void Plugin::SecondInit ()
	{
		for (const auto mgr : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IRemovableDevManager*> ())
			if (mgr->SupportsDevType (USBDevice))
			{
				const auto model = mgr->GetDevicesModel ();
				connect (model,
						&QAbstractItemModel::rowsInserted,
						this,
						[this, model] (const QModelIndex& idx, int from, int to)
						{
							Mtp_->Run (&Mtp::HandleDevicesConnected, GetModelDevices (*model, idx, from, to));
						});
				connect (model,
						&QAbstractItemModel::rowsAboutToBeRemoved,
						this,
						[this, model] (const QModelIndex& idx, int from, int to)
						{
							Mtp_->Run (&Mtp::HandleDevicesDisconnected, GetModelDevices (*model, idx, from, to));
						});
			}
	}

	void Plugin::Release ()
	{
		Mtp_.reset ();
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.MTPSync"_qba;
	}

	QString Plugin::GetName () const
	{
		return "LMP MTPSync"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for synchronization with MTP-enabled portable media players.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.LMP.CollectionSync"_qba };
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	QString Plugin::GetSyncSystemName () const
	{
		return "MTP"_qs;
	}

	QAbstractItemModel& Plugin::GetSyncTargetsModel () const
	{
		return *DevicesModel_;
	}

	ISyncPluginConfigWidget_ptr Plugin::MakeConfigWidget ()
	{
		return {};
	}

	void Plugin::RefreshSyncTargets ()
	{
		Mtp_->Run (&Mtp::Refresh);
	}

	Util::ContextTask<ISyncPlugin::UploadResult> Plugin::Upload (UploadJob job)
	{
		const auto& serial = job.Target_.data (CommonDevRole::DevPersistentID).toByteArray ();
		return Mtp_->Run (&Mtp::Upload, Mtp::UploadCtx {
					.Serial_ = serial,
					.StorageId_ = 0,
					.LocalPath_ = job.LocalPath_,
					.MediaInfo_ = job.MediaInfo_,
					.AlbumArtPath_ = LMPProxy_->GetUtilProxy ()->FindAlbumArt (job.OriginalLocalPath_),
				});
	}

	void Plugin::AddDevices (const QList<MtpDeviceInfo>& devices)
	{
		QList<ModelRow> rows;
		for (const auto& dev : devices)
		{
			if (!std::ranges::any_of (DevicesModel_->GetItems (),
					[&dev] (const ModelRow& row) { return row.Serial_ == dev.Serial_; }))
				rows << ModelRow { dev.Serial_, dev.DevName_ };
		}
		DevicesModel_->AddItems (rows);
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_mtpsync, LC::LMP::MTPSync::Plugin);
