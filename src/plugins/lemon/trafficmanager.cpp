/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "trafficmanager.h"
#include <QStandardItemModel>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QTimer>
#include <util/util.h>
#include <util/models/rolenamesmixin.h>
#include "core.h"
#include "platformbackend.h"

namespace LC
{
namespace Lemon
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

			IfacesModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::IfaceName] = "ifaceName";
				roleNames [Roles::BearerType] = "bearerType";
				roleNames [Roles::IconName] = "iconName";
				roleNames [Roles::UpSpeed] = "upSpeed";
				roleNames [Roles::UpSpeedPretty] = "upSpeedPretty";
				roleNames [Roles::DownSpeed] = "downSpeed";
				roleNames [Roles::DownSpeedPretty] = "downSpeedPretty";
				roleNames [Roles::MaxUpSpeed] = "maxUpSpeed";
				roleNames [Roles::MaxDownSpeed] = "maxDownSpeed";
				setRoleNames (roleNames);
			}
		};
	}

	TrafficManager::TrafficManager (QObject *parent)
	: QObject (parent)
	, Model_ (new IfacesModel (this))
	, ConfManager_ (new QNetworkConfigurationManager (this))
	{
		connect (ConfManager_,
				SIGNAL (configurationAdded (QNetworkConfiguration)),
				this,
				SLOT (addConfiguration (QNetworkConfiguration)));
		connect (ConfManager_,
				SIGNAL (configurationRemoved (QNetworkConfiguration)),
				this,
				SLOT (removeConfiguration (QNetworkConfiguration)));
		connect (ConfManager_,
				SIGNAL (configurationChanged (QNetworkConfiguration)),
				this,
				SLOT (handleConfigChanged (QNetworkConfiguration)));

		ConfManager_->updateConfigurations ();

		for (const auto& conf : ConfManager_->allConfigurations (QNetworkConfiguration::Active))
			addConfiguration (conf);

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT (updateCounters ()));
		timer->start (1000);
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

	namespace
	{
		struct NetIcons
		{
			QMap<QNetworkConfiguration::BearerType, QString> Icons_;

			NetIcons ()
			{
				Icons_ [QNetworkConfiguration::BearerEthernet] = "network-wired";
				Icons_ [QNetworkConfiguration::BearerWLAN] = "network-wireless";
				Icons_ [QNetworkConfiguration::BearerWiMAX] = "network-wireless";

				Icons_ [QNetworkConfiguration::Bearer2G] = "mobile";
				Icons_ [QNetworkConfiguration::BearerCDMA2000] = "mobile";
				Icons_ [QNetworkConfiguration::BearerWCDMA] = "mobile";
				Icons_ [QNetworkConfiguration::BearerHSPA] = "mobile";

				Icons_ [QNetworkConfiguration::BearerUnknown] = "network-workgroup";
			}
		};
	}

	void TrafficManager::addConfiguration (const QNetworkConfiguration& conf)
	{
		static NetIcons icons;

		const auto sess = std::make_shared<QNetworkSession> (conf);
		if (sess->state () != QNetworkSession::Connected)
			return;

		auto iface = sess->interface ();
		const auto& ifaceId = iface.name ();
		const auto& config = sess->configuration ();

		if (!ActiveInterfaces_.contains (ifaceId))
		{
			auto item = new QStandardItem;
			Model_->appendRow (item);

			InterfaceInfo info (item);
			info.Name_ = ifaceId;

			auto backend = Core::Instance ().GetPlatformBackend ();
			if (backend)
			{
				backend->update ({ ifaceId });
				const auto& bytesStats = backend->GetCurrentNumBytes (ifaceId);
				info.PrevRead_ = bytesStats.Down_;
				info.PrevWritten_ = bytesStats.Up_;
			}

			ActiveInterfaces_ [ifaceId] = info;
		}

		auto& info = ActiveInterfaces_ [ifaceId];

		auto item = info.Item_;
		item->setData (iface.humanReadableName (), IfacesModel::Roles::IfaceName);
		item->setData (config.bearerTypeName (), IfacesModel::Roles::BearerType);
		item->setData (icons.Icons_ [config.bearerType ()], IfacesModel::Roles::IconName);
		item->setData (0, IfacesModel::Roles::MaxDownSpeed);
		item->setData (0, IfacesModel::Roles::MaxUpSpeed);

		info.LastSession_ = sess;
	}

	void TrafficManager::removeConfiguration (const QNetworkConfiguration& conf)
	{
		for (const auto& info : ActiveInterfaces_)
		{
			if (info.LastSession_->configuration () != conf)
				continue;

			Model_->removeRow (info.Item_->row ());
			ActiveInterfaces_.remove (info.Name_);
			break;
		}
	}

	void TrafficManager::handleConfigChanged (const QNetworkConfiguration& conf)
	{
		if (conf.state () == QNetworkConfiguration::Active)
			addConfiguration (conf);
		else
			removeConfiguration (conf);
	}

	void TrafficManager::updateCounters ()
	{
		auto backend = Core::Instance ().GetPlatformBackend ();
		if (!backend)
			return;

		backend->update (ActiveInterfaces_.keys ());

		const auto backtrack = GetBacktrackSize ();

		for (auto& info : ActiveInterfaces_)
		{
			const auto& name = info.Name_;

			const auto& bytesStats = backend->GetCurrentNumBytes (name);

			auto updateCounts = [&info, backtrack] (const qint64 now, qint64& prev,
					QVector<qint64>& list, IfacesModel::Roles role, const QString& text) -> qint64
			{
				const auto diff = now - prev;

				info.Item_->setData (diff, role);
				info.Item_->setData (text.arg (Util::MakePrettySize (diff)), role + 1);

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

			auto updateMax = [&info] (const QVector<qint64>& speeds, IfacesModel::Roles role)
			{
				const auto max = *std::max_element (speeds.begin (), speeds.end ());
				info.Item_->setData (max, role);
			};
			updateMax (info.DownSpeeds_, IfacesModel::Roles::MaxDownSpeed);
			updateMax (info.UpSpeeds_, IfacesModel::Roles::MaxUpSpeed);
		}

		emit updated ();
	}
}
}
