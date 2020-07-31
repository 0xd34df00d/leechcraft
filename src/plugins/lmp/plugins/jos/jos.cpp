/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "jos.h"
#include <QIcon>
#include "devmanager.h"
#include "uploadmanager.h"

namespace LC
{
namespace LMP
{
namespace jOS
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		DevManager_ = new DevManager (this);
		connect (DevManager_,
				SIGNAL (availableDevicesChanged ()),
				this,
				SIGNAL (availableDevicesChanged ()));

		UpManager_ = new UploadManager (this);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.jOS";
	}

	QString Plugin::GetName () const
	{
		return "LMP jOS";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for synchronization with iOS-based devices like iPhone, iPod Touch and iPad.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.CollectionSync";
		return result;
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}

	QString Plugin::GetSyncSystemName () const
	{
		return "iOS";
	}

	QObject* Plugin::GetQObject ()
	{
		return this;
	}

	UnmountableDevInfos_t Plugin::AvailableDevices () const
	{
		return DevManager_->GetDevices ();
	}

	void Plugin::SetFileInfo (const QString& origLocalPath, const UnmountableFileInfo& info)
	{
		UpManager_->SetInfo (origLocalPath, info);
	}

	void Plugin::Upload (const QString& localPath, const QString& origLocalPath, const QByteArray& to, const QByteArray&)
	{
		UpManager_->Upload (localPath, origLocalPath, to);
	}

	void Plugin::Refresh ()
	{
		DevManager_->refresh ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_jos, LC::LMP::jOS::Plugin);
