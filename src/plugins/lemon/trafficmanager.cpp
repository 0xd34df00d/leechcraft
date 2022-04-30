/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trafficmanager.h"
#include <QNetworkInterface>
#include <QStandardItemModel>
#include <QTimer>
#include <util/util.h>
#include <util/models/rolenamesmixin.h>
#include <util/sll/qtutil.h>
#include "platformbackend.h"

namespace LC::Lemon
{
	namespace
	{
		class IfacesModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				IfaceName = Qt::UserRole + 1,
				BearerType,
				IconName,
				UpSpeed,
				UpSpeedPretty,
				DownSpeed,
				DownSpeedPretty,
				MaxUpSpeed,
				MaxDownSpeed
			};

			explicit IfacesModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> { parent }
			{
				setRoleNames ({
						{ Roles::IfaceName, "ifaceName" },
						{ Roles::BearerType, "bearerType" },
						{ Roles::IconName, "iconName" },
						{ Roles::UpSpeed, "upSpeed" },
						{ Roles::UpSpeedPretty, "upSpeedPretty" },
						{ Roles::DownSpeed, "downSpeed" },
						{ Roles::DownSpeedPretty, "downSpeedPretty" },
						{ Roles::MaxUpSpeed, "maxUpSpeed" },
						{ Roles::MaxDownSpeed, "maxDownSpeed" },
					});
			}
		};
	}

	TrafficManager::TrafficManager (std::shared_ptr<PlatformBackend> backend, QObject *parent)
	: QObject { parent }
	, Model_ { new IfacesModel { this } }
	, Backend_ { std::move (backend) }
	{
		auto timer = new QTimer (this);
		timer->callOnTimeout ([this]
			{
				UpdateInterfaces ();
				UpdateCounters ();

				emit updated ();
			});
		timer->start (1000);
		timer->setTimerType (Qt::VeryCoarseTimer);
	}

	QAbstractItemModel* TrafficManager::GetModel () const
	{
		return Model_;
	}

	QVector<qint64> TrafficManager::GetDownHistory (const QString& name) const
	{
		return ActiveInterfaces_ [name].DownSpeeds_;
	}

	QVector<qint64> TrafficManager::GetUpHistory (const QString& name) const
	{
		return ActiveInterfaces_ [name].UpSpeeds_;
	}

	int TrafficManager::GetBacktrackSize () const
	{
		return 500;
	}

	void TrafficManager::AddInterface (const QNetworkInterface& iface)
	{
		const auto& name = iface.name ();

		InterfaceInfo info { .Item_ = new QStandardItem };

		if (Backend_)
		{
			Backend_->Update ({ name });
			const auto& bytesStats = Backend_->GetCurrentNumBytes (name);
			info.PrevRead_ = bytesStats.Down_;
			info.PrevWritten_ = bytesStats.Up_;
		}

		ActiveInterfaces_ [name] = info;

		Model_->appendRow (info.Item_);
	}

	void TrafficManager::UpdateInterface (const QNetworkInterface& iface)
	{
		auto item = ActiveInterfaces_ [iface.name ()].Item_;
		if (!item)
		{
			qCritical () << Q_FUNC_INFO
					<< "unknown interface"
					<< iface
					<< "in"
					<< ActiveInterfaces_.keys ();
			return;
		}

		struct NetInfo
		{
			QString Icon_;
			QString TypeStr_;
		};
		static const QMap<QNetworkInterface::InterfaceType, NetInfo> netInfos
		{
			{ QNetworkInterface::Ethernet, { "network-wired", tr ("Ethernet") } },
			{ QNetworkInterface::Virtual, { "network-server", tr ("Virtual interface") } },
			{ QNetworkInterface::Wifi, { "network-wireless", "WiFi" } },
			{ QNetworkInterface::Ppp, { "network-wired", "PPP" } },
		};

		const auto netInfo = netInfos.value (iface.type (), { .Icon_ = "network-workgroup", .TypeStr_ = tr ("Unknown type") });

		item->setData (iface.humanReadableName (), IfacesModel::Roles::IfaceName);
		item->setData (netInfo.TypeStr_, IfacesModel::Roles::BearerType);
		item->setData (netInfo.Icon_, IfacesModel::Roles::IconName);
		item->setData (0, IfacesModel::Roles::MaxDownSpeed);
		item->setData (0, IfacesModel::Roles::MaxUpSpeed);
	}

	void TrafficManager::RemoveInterface (const QString& name)
	{
		const auto& info = ActiveInterfaces_.take (name);
		if (info.Item_)
			Model_->removeRow (info.Item_->row ());
	}

	void TrafficManager::UpdateInterfaces ()
	{
		QSet<QString> currentNames { ActiveInterfaces_.keyBegin (), ActiveInterfaces_.keyEnd () };

		for (const auto& iface : QNetworkInterface::allInterfaces ())
		{
			if (iface.flags () & QNetworkInterface::IsLoopBack)
				continue;

			const auto& name = iface.name ();
			currentNames.remove (name);

			const auto isActive = iface.flags () & QNetworkInterface::IsRunning;
			const auto isKnown = ActiveInterfaces_.contains (iface.name ());
			if (isActive)
			{
				if (!isKnown)
					AddInterface (iface);
				UpdateInterface (iface);
			}
			else if (!isActive && isKnown)
				RemoveInterface (name);
		}

		for (const auto& deleted : currentNames)
			RemoveInterface (deleted);
	}

	void TrafficManager::UpdateCounters ()
	{
		if (!Backend_)
			return;

		Backend_->Update (ActiveInterfaces_.keys ());

		const auto backtrack = GetBacktrackSize ();

		for (auto&& [name, info] : Util::Stlize (ActiveInterfaces_))
		{
			const auto& bytesStats = Backend_->GetCurrentNumBytes (name);

			auto updateCounts = [backtrack, item = info.Item_] (const qint64 now, qint64& prev,
					QVector<qint64>& list, IfacesModel::Roles role, const QString& text)
			{
				const auto diff = now - prev;

				item->setData (diff, role);
				item->setData (text.arg (Util::MakePrettySize (diff)), role + 1);

				list << diff;
				if (list.size () > backtrack)
					list.erase (list.begin ());

				prev = now;
				return diff;
			};

			updateCounts (bytesStats.Down_, info.PrevRead_, info.DownSpeeds_,
					IfacesModel::Roles::DownSpeed, tr ("Download speed: %1/s"));
			updateCounts (bytesStats.Up_, info.PrevWritten_, info.UpSpeeds_,
					IfacesModel::Roles::UpSpeed, tr ("Upload speed: %1/s"));

			auto updateMax = [item = info.Item_] (const QVector<qint64>& speeds, IfacesModel::Roles role)
			{
				const auto max = *std::max_element (speeds.begin (), speeds.end ());
				item->setData (max, role);
			};
			updateMax (info.DownSpeeds_, IfacesModel::Roles::MaxDownSpeed);
			updateMax (info.UpSpeeds_, IfacesModel::Roles::MaxUpSpeed);
		}
	}
}
