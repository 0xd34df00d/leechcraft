/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "devmon.h"
#include <QIcon>
#include <util/util.h>
#include "udevbackend.h"

namespace LC
{
namespace Devmon
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("devmon");

		Backend_ = new UDevBackend (proxy);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Devmon";
	}

	void Plugin::Release ()
	{
		delete Backend_;
	}

	QString Plugin::GetName () const
	{
		return "Devmon";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("USB non-storage devices monitor plugin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	bool Plugin::SupportsDevType (DeviceType type) const
	{
		return type == DeviceType::USBDevice;
	}

	QAbstractItemModel* Plugin::GetDevicesModel () const
	{
		return Backend_->GetModel ();
	}

	void Plugin::MountDevice (const QString&)
	{
		qWarning () << Q_FUNC_INFO
				<< "mounts aren't suported";
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_devmon, LC::Devmon::Plugin);

