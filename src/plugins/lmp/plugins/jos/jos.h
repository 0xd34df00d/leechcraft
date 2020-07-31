/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/isyncplugin.h>
#include <interfaces/lmp/iunmountablesync.h>

namespace LC
{
namespace LMP
{
namespace jOS
{
	class DevManager;
	class UploadManager;

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

		LC_PLUGIN_METADATA ("org.LeechCraft.LMP.jOS")

		ILMPProxy_ptr LMPProxy_;

		DevManager *DevManager_;
		UploadManager *UpManager_;
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
		void SetFileInfo (const QString&, const UnmountableFileInfo&);
		void Upload (const QString&, const QString&, const QByteArray&, const QByteArray&);
		void Refresh ();
	signals:
		void availableDevicesChanged ();
		void uploadProgress (qint64, qint64);
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};
}
}
}

