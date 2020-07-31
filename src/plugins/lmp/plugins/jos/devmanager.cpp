/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "devmanager.h"
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <QTimer>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QFuture>
#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <util/sll/delayedexecutor.h>
#include "mobileraii.h"

namespace LC
{
namespace LMP
{
namespace jOS
{
	DevManager::DevManager (QObject *parent)
	: QObject (parent)
	{
		idevice_event_subscribe ([] (const idevice_event_t*, void *mgr)
				{
					Util::ExecuteLater ([mgr] { static_cast<DevManager*> (mgr)->refresh (); });
				},
				this);
	}

	UnmountableDevInfos_t DevManager::GetDevices () const
	{
		return Devices_;
	}

	namespace
	{
		template<typename PListGetter>
		struct PListType;

		template<typename Res>
		struct PListType<void (*) (plist_t, Res*)>
		{
			typedef Res type;
		};

		template<typename PListGetter>
		auto Wrap (PListGetter g)
		{
			return [g] (plist_t t)
			{
				typename PListType<PListGetter>::type r;
				g (t, &r);
				return r;
			};
		}

		template<typename L>
		QVariant GetProperty (const L& lockdown, const QString& prop, const QString& domain = QString ())
		{
			const auto& plist = MakeRaii<plist_t> ([&] (plist_t *plist)
					{
						return lockdownd_get_value (lockdown,
								domain.isEmpty () ? nullptr : domain.toUtf8 ().constData (),
								prop.toUtf8 ().constData (),
								plist);
					},
					plist_free);

			switch (plist_get_node_type (plist))
			{
			case PLIST_NONE:
				return {};
			case PLIST_BOOLEAN:
				return static_cast<bool> (Wrap (plist_get_bool_val) (plist));
			case PLIST_UINT:
				return static_cast<quint64> (Wrap (plist_get_uint_val) (plist));
			case PLIST_STRING:
			{
				std::shared_ptr<char> str { Wrap (plist_get_string_val) (plist), free };
				return QString::fromUtf8 (str.get ());
			}
			default:
				qWarning () << Q_FUNC_INFO
						<< "unhandled type"
						<< plist_get_node_type (plist);
				return {};
			}
		}

		UnmountableDevInfo QueryDevice (const char *udid)
		{
			const auto& device = MakeRaii<idevice_t> ([udid] (idevice_t *dev)
						{ return idevice_new (dev, udid); },
					idevice_free);

			const auto& lockdown = MakeRaii<lockdownd_client_t> ([&device] (lockdownd_client_t *client)
						{ return lockdownd_client_new_with_handshake (device, client, "LMP jOS"); },
					lockdownd_client_free);

			const auto& id = GetProperty (lockdown, "UniqueDeviceID").toByteArray ();
			auto name = GetProperty (lockdown, "DeviceName").toString ();
			const auto available = GetProperty (lockdown, "TotalDataAvailable", "com.apple.disk_usage").value<quint64> ();
			const auto total = GetProperty (lockdown, "TotalDataCapacity", "com.apple.disk_usage").value<quint64> ();

			const auto& type = GetProperty (lockdown, "ProductType").toString ();
			const auto& osVersion = GetProperty (lockdown, "ProductVersion").toString ();

			if (!type.isEmpty ())
				name += " " + type;
			if (!osVersion.isEmpty ())
				name += " (iOS " + osVersion + ")";

			return
			{
				id,
				"Apple",
				name,
				{
					{
						"RootPartition",
						DevManager::tr ("Root partition"),
						available,
						total
					}
				},
				{ "aac", "alac", "mp3" },
				-1
			};
		}

		UnmountableDevInfos_t QueryDevices ()
		{
			UnmountableDevInfos_t result;

			char **devices = nullptr;
			int count = 0;
			idevice_get_device_list (&devices, &count);

			qDebug () << "found" << count << "devices";
			for (int i = 0; i < count; ++i)
			{
				qDebug () << devices [i];

				try
				{
					result << QueryDevice (devices [i]);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< "cannot query"
							<< devices [i]
							<< "because:"
							<< e.what ();
				}
			}

			idevice_device_list_free (devices);

			return result;
		}
	}

	void DevManager::refresh ()
	{
		if (PollWatcher_)
		{
			qWarning () << Q_FUNC_INFO
					<< "already refreshing";
			return;
		}

		PollWatcher_ = new QFutureWatcher<UnmountableDevInfos_t> ();
		connect (PollWatcher_,
				SIGNAL (finished ()),
				this,
				SLOT (handlePolled ()));

		auto future = QtConcurrent::run (QueryDevices);
		PollWatcher_->setFuture (future);
	}

	void DevManager::handlePolled ()
	{
		qDebug () << Q_FUNC_INFO;
		auto result = PollWatcher_->result ();
		PollWatcher_->deleteLater ();
		PollWatcher_ = nullptr;

		std::sort (result.begin (), result.end ());

		if (result == Devices_)
			return;

		Devices_ = result;
		emit availableDevicesChanged ();
	}
}
}
}
