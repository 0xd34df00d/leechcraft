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

#include "mtpsync.h"
#include <QIcon>
#include <QTimer>

namespace LeechCraft
{
namespace LMP
{
namespace MTPSync
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		LIBMTP_Init ();

		auto timer = new QTimer ();
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (pollDevices ()));
		timer->start (5000);
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.MTPSync";
	}

	QString Plugin::GetName () const
	{
		return "LMP MTPSync";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for synchronization with MTP-enabled portable media players.");
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
		return "MTP";
	}

	UnmountableDevInfos_t Plugin::AvailableDevices () const
	{
		return UnmountableDevInfos_t ();
	}

	void Plugin::Upload (const QString& localPath, const QString& origLocalPath, const QByteArray& devId)
	{
	}

	void Plugin::pollDevices ()
	{
		LIBMTP_mtpdevice_t *devices;
		qDebug () << LIBMTP_Get_Connected_Devices (&devices);
		qDebug () << LIBMTP_Number_Devices_In_List (devices);
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_mtpsync, LeechCraft::LMP::MTPSync::Plugin);
