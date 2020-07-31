/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QDateTime>
#include <libmtp.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/iunmountablesync.h>

class QAbstractItemModel;
class QModelIndex;

namespace LC
{
namespace LMP
{
namespace MTPSync
{
	struct USBDevInfo
	{
		UnmountableDevInfo Info_;
		int Busnum_;
		int Devnum_;
	};
	typedef QList<USBDevInfo> USBDevInfos_t;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public ILMPPlugin
				 , public IUnmountableSync
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LC::LMP::ILMPPlugin
				LC::LMP::IUnmountableSync)

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.MTPSync")

		ICoreProxy_ptr Proxy_;
		ILMPProxy_ptr LMPProxy_ = {};
		USBDevInfos_t Infos_;

		QHash<QString, UnmountableFileInfo> OrigInfos_;

		struct DeviceCacheEntry
		{
			std::shared_ptr<LIBMTP_mtpdevice_t> Device_;
		};
		QHash<QByteArray, DeviceCacheEntry> DevicesCache_;

		struct UploadQueueItem
		{
			QString LocalPath_;
			QString OrigLocalPath_;
			QByteArray To_;
			QByteArray StorageID_;
		};
		QList<UploadQueueItem> UploadQueue_;

		QAbstractItemModel *Model_ = nullptr;

		bool FirstPoll_ = true;
		bool IsPolling_ = false;
		bool IsUploading_ = false;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		void SetLMPProxy (ILMPProxy_ptr);

		QString GetSyncSystemName () const;
		QObject* GetQObject ();
		UnmountableDevInfos_t AvailableDevices () const;
		void SetFileInfo (const QString& origLocalPath, const UnmountableFileInfo& info);
		void Upload (const QString& localPath, const QString& origLocalPath, const QByteArray& to, const QByteArray& storageId);
		void Refresh ();

		void HandleTransfer (quint64, quint64);
	private:
		void UploadTo (LIBMTP_mtpdevice_t*, const QByteArray&, const QString&, const QString&);
		void AppendAlbum (LIBMTP_mtpdevice_t*, LIBMTP_track_t*, const UnmountableFileInfo&);

		void Subscribe2Devs ();
	private slots:
		void handleUploadFinished ();
		void pollDevices ();
		void handlePollFinished ();

		void handleRowsInserted (const QModelIndex&, int, int);
		void handleRowsRemoved (const QModelIndex&, int, int);
	signals:
		void availableDevicesChanged ();
		void uploadProgress (qint64, qint64);
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};
}
}
}
