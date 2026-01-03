/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "dumbsync.h"
#include <memory>
#include <QBindable>
#include <QConcatenateTablesProxyModel>
#include <QFileInfo>
#include <QDir>
#include <QIcon>
#include <QLineEdit>
#include <QtConcurrentRun>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/devices/deviceroles.h>
#include <interfaces/devices/iremovabledevmanager.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro.h>
#include <util/util.h>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/lmp/ilmpproxy.h>
#include <interfaces/lmp/ilmputilproxy.h>
#include <util/lmp/util.h>

using QFile_ptr = std::shared_ptr<QFile>;

namespace LC::LMP::DumbSync
{
	using XmlSettingsManager = Util::SingletonSettingsManager<"LMP_DumbSync">;

	namespace
	{
		class MountableFilterModel : public QSortFilterProxyModel
		{
		public:
			using QSortFilterProxyModel::QSortFilterProxyModel;

			QVariant data (const QModelIndex& index, int role) const override
			{
				if (role != Qt::DisplayRole)
					return QSortFilterProxyModel::data (index, role);

				const auto& srcIdx = mapToSource (index);

				const auto& mounts = srcIdx.data (MassStorageRole::MountPoints).toStringList ();
				const auto& mountText = mounts.isEmpty () ?
						Plugin::tr ("not mounted") :
						Plugin::tr ("mounted at %1").arg (mounts.join ("; "));

				const auto& size = srcIdx.data (MassStorageRole::TotalSize).toLongLong ();
				return QString ("%1 (%2, %3), %4")
						.arg (srcIdx.data (MassStorageRole::VisibleName).toString ())
						.arg (Util::MakePrettySize (size))
						.arg (srcIdx.data (MassStorageRole::DevFile).toString ())
						.arg (mountText);
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex&) const override
			{
				return sourceModel()->index (row, 0).data (MassStorageRole::IsMountable).toBool ();
			}
		};
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "lmpdumbsyncsettings.xml"_qs);

		SyncTargets_ = std::make_unique<MountableFilterModel> ();
	}

	void Plugin::SecondInit ()
	{
		const auto merger = new QConcatenateTablesProxyModel { this };

		const auto pm = GetProxyHolder ()->GetPluginsManager ();
		for (const auto& mgr : pm->GetAllCastableTo<IRemovableDevManager*> ())
			if (mgr->SupportsDevType (DeviceType::MassStorage))
				merger->addSourceModel (mgr->GetDevicesModel ());

		SyncTargets_->setSourceModel (merger);
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.DumbSync"_qba;
	}

	QString Plugin::GetName () const
	{
		return "LMP DumbSync"_qs;
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Music collection synchronization with Mass Storage-like devices, like USB Flash drives and Rockbox players.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
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
		return tr ("dumb copying");
	}

	QAbstractItemModel& Plugin::GetSyncTargetsModel () const
	{
		return *SyncTargets_;
	}

	void Plugin::RefreshSyncTargets ()
	{
	}

	namespace
	{
		struct UploadConfig : ISyncPluginConfig
		{
			QString Mask_;

			explicit UploadConfig (QString mask)
			: Mask_ { std::move (mask) }
			{
			}
		};
	}

	ISyncPluginConfigWidget_ptr Plugin::MakeConfigWidget ()
	{
		class ConfigWidget : public QLineEdit
						   , public ISyncPluginConfigWidget
		{
		public:
			explicit ConfigWidget ()
			{
				const auto& defaultMask = "music/$artist/$year $album/$trackNumber - $title"_qs;
				setText (XmlSettingsManager::Instance ().Property ("FileMask", defaultMask).toString ());
			}

			QWidget* GetQWidget () override
			{
				return this;
			}

			ISyncPluginConfig_cptr GetConfig () const override
			{
				return std::make_shared<const UploadConfig> (text ());
			}
		};

		return std::make_unique<ConfigWidget> ();
	}

	namespace
	{
		QString FixMask (const MediaInfo& info, const QString& mask)
		{
			auto result = PerformSubstitutions (mask, info, SubstitutionFlag::SFSafeFilesystem);
			const auto& ext = QFileInfo { info.LocalPath_ }.suffix ();
			if (!result.endsWith (ext))
				result += "." + ext;
			return result;
		}

		QImage GetScaledPixmap (const QString& pxFile)
		{
			if (pxFile.isEmpty ())
				return {};

			QImage img { pxFile };
			const int maxDim = XmlSettingsManager::Instance ().property ("CoverDim").toInt ();
			if (img.size ().width () <= maxDim && img.size ().height () <= maxDim)
				return img;

			return img.scaled (maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		void WriteScaledPixmap (const QString& pxFile, const QString& target)
		{
			const auto& coverName = XmlSettingsManager::Instance ().property ("CoverName").toString ();

			const auto& targetDir = QFileInfo { target }.absoluteDir ();
			if (targetDir.exists (coverName))
				return;

			const auto& px = GetScaledPixmap (pxFile);
			if (px.isNull ())
				return;

			constexpr auto aaQuality = 80;
			if (const auto& fullCoverPath = targetDir.absoluteFilePath (coverName);
				!px.save (fullCoverPath, "JPG", aaQuality))
				qWarning () << "unable to save album art from" << pxFile << "to" << fullCoverPath;
		}
	}

	Util::ContextTask<Plugin::UploadResult> Plugin::Upload (UploadJob job)
	{
		const auto& config = dynamic_cast<const UploadConfig&> (*job.Config_);
		XmlSettingsManager::Instance ().setProperty ("FileMask", config.Mask_);

		const auto& targetRelPath = FixMask (job.MediaInfo_, config.Mask_);
		const auto& targetMountPoint = job.Target_.data (MassStorageRole::MountPoints).toStringList ().value (0);
		if (targetMountPoint.isEmpty ())
		{
			qWarning () << "bad target" << job.Target_;
			co_return UploadFailure
			{
				QFile::OpenError,
				tr ("Target mount point does not exist."),
			};
		}

		auto target = targetMountPoint;
		if (!target.endsWith ('/') && !targetRelPath.startsWith ('/'))
			target += '/';
		target += targetRelPath;
		qDebug () << "uploading" << job.LocalPath_ << "(from" << job.OriginalLocalPath_ << ") to " << target;

		const QString& dirPath = targetRelPath.section ('/', 0, -2);
		if (!QDir (targetMountPoint).mkpath (dirPath))
			co_return UploadFailure
			{
				QFile::PermissionsError,
				tr ("Unable to create the directory path %1 on target device %2.").arg (dirPath, targetMountPoint)
			};

		const auto& artPath = LMPProxy_->GetUtilProxy ()->FindAlbumArt (job.OriginalLocalPath_);

		co_return co_await QtConcurrent::run ([&] () -> UploadResult
				{
					if (QFile file { job.LocalPath_ };
						!file.copy (target))
					{
						qWarning () << "failed to copy" << job.LocalPath_ << "to" << target << ":" << file.error () << file.errorString ();
						return { Util::AsLeft, { file.error (), file.errorString () } };
					}
					if (XmlSettingsManager::Instance ().property ("UploadCovers").toBool ())
						WriteScaledPixmap (artPath, target);
					return UploadSuccess {};
				});
	}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_dumbsync, LC::LMP::DumbSync::Plugin);
