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

#include "modelmanager.h"
#include <QStandardItemModel>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QTimer>
#include "core.h"
#include "platformbackend.h"

namespace LeechCraft
{
namespace Lemon
{
	namespace
	{
		class IfacesModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				IfaceName = Qt::UserRole + 1,
				BearerType,
				IconName,
				IsActive
			};

			IfacesModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::IfaceName] = "ifaceName";
				roleNames [Roles::BearerType] = "bearerType";
				roleNames [Roles::IconName] = "iconName";
				roleNames [Roles::IsActive] = "isActive";
				setRoleNames (roleNames);
			}
		};
	}

	ModelManager::ModelManager (QObject *parent)
	: QObject (parent)
	, Model_ (new IfacesModel (this))
	, ConfManager_ (new QNetworkConfigurationManager (this))
	{
		connect (ConfManager_,
				SIGNAL (configurationAdded (QNetworkConfiguration)),
				this,
				SLOT (addConfiguration (QNetworkConfiguration)));

		ConfManager_->updateConfigurations ();

		for (const auto& conf : ConfManager_->allConfigurations (QNetworkConfiguration::Active))
			addConfiguration (conf);

		auto timer = new QTimer (this);
		connect (timer,
				SIGNAL (timeout ()),
				this,
				SLOT(updateCounters ()));
		timer->start (1000);
	}

	QAbstractItemModel* ModelManager::GetModel () const
	{
		return Model_;
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

	void ModelManager::addConfiguration (const QNetworkConfiguration& conf)
	{
		static NetIcons icons;

		QNetworkSession_ptr sess (new QNetworkSession (conf, this));
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
			ActiveInterfaces_ [ifaceId] = info;
		}

		auto& info = ActiveInterfaces_ [ifaceId];

		auto item = info.Item_;
		item->setData (iface.humanReadableName (), IfacesModel::Roles::IfaceName);
		item->setData (config.bearerTypeName (), IfacesModel::Roles::BearerType);
		item->setData (icons.Icons_ [config.bearerType ()], IfacesModel::Roles::IconName);
		item->setData (false, IfacesModel::Roles::IsActive);

		info.LastSession_ = sess;
		info.PrevRead_ = sess->bytesReceived ();
		info.PrevWritten_ = sess->bytesWritten ();
	}

	void ModelManager::updateCounters ()
	{
		auto backend = Core::Instance ().GetPlatformBackend ();
		if (!backend)
			return;

		backend->update (ActiveInterfaces_.keys ());

		auto updateCounts = [] (const qint64 now, qint64& prev, QList<qint64>& list) -> qint64
		{
			const auto diff = now - prev;
			list << diff;
			if (list.size () > 500)
				list.erase (list.begin ());
			prev = now;
			return diff;
		};

		for (auto& info : ActiveInterfaces_)
		{
			const auto& name = info.Name_;

			const auto& bytesStats = backend->GetCurrentNumBytes (name);

			auto readDiff = updateCounts (bytesStats.Down_, info.PrevRead_, info.DownSpeeds_);
			auto writeDiff = updateCounts (bytesStats.Up_, info.PrevWritten_, info.UpSpeeds_);

			qDebug () << name << readDiff << writeDiff;
			info.Item_->setData (readDiff > 3 * 1024 || writeDiff > 3 * 1024,
					IfacesModel::Roles::IsActive);
		}
	}
}
}
