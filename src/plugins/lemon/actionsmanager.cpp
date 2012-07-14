/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "actionsmanager.h"
#include <QNetworkConfigurationManager>
#include <QNetworkSession>
#include <QAction>
#include "core.h"

namespace LeechCraft
{
namespace Lemon
{
	ActionsManager::InterfaceInfo::InterfaceInfo ()
	: Action_ (0)
	, PrevRead_ (0)
	, PrevWritten_ (0)
	{
	}

	ActionsManager::ActionsManager (QObject *parent)
	: QObject (parent)
	, Manager_ (new QNetworkConfigurationManager (this))
	{
		connect (Manager_,
				SIGNAL (configurationAdded (QNetworkConfiguration)),
				this,
				SLOT (addConfiguration (QNetworkConfiguration)));

		Manager_->updateConfigurations ();

		for (const auto& conf : Manager_->allConfigurations (QNetworkConfiguration::Active))
			addConfiguration (conf);
	}

	QList<QAction*> ActionsManager::GetActions() const
	{
		QList<QAction*> result;
		for (const auto& info : Infos_.values ())
			result << info.Action_;
		return result;
	}

	namespace
	{
		struct NetIcons
		{
			QMap<QNetworkConfiguration::BearerType, QIcon> Icons_;

			NetIcons ()
			{
				auto p = Core::Instance ().GetProxy ();
				Icons_ [QNetworkConfiguration::BearerEthernet] = p->GetIcon ("network-wired");
				Icons_ [QNetworkConfiguration::BearerWLAN] = p->GetIcon ("network-wireless");
				Icons_ [QNetworkConfiguration::BearerWiMAX] = p->GetIcon ("network-wireless");

				const auto& mobile = p->GetIcon ("mobile");
				Icons_ [QNetworkConfiguration::Bearer2G] = mobile;
				Icons_ [QNetworkConfiguration::BearerCDMA2000] = mobile;
				Icons_ [QNetworkConfiguration::BearerWCDMA] = mobile;
				Icons_ [QNetworkConfiguration::BearerHSPA] = mobile;

				Icons_ [QNetworkConfiguration::BearerUnknown] = p->GetIcon ("network-workgroup");
			}
		};
	}

	void ActionsManager::addConfiguration (const QNetworkConfiguration& conf)
	{
		static NetIcons icons;

		QNetworkSession_ptr sess (new QNetworkSession (conf, this));
		if (sess->state () != QNetworkSession::Connected)
			return;

		auto iface = sess->interface ();
		const auto& ifaceId = iface.hardwareAddress ();
		const auto& config = sess->configuration ();

		if (!Infos_.contains (ifaceId))
		{
			const auto& title = tr ("Network interface: %1 (%2)")
					.arg (iface.humanReadableName ())
					.arg (config.bearerTypeName ());
			auto action = new QAction (title, this);
			action->setIcon (icons.Icons_ [config.bearerType ()]);
			Infos_ [ifaceId].Action_ = action;
			emit gotActions ({ action }, ActionsEmbedPlace::LCTray);
		}

		Infos_ [ifaceId].Sessions_ [conf.name ()] = sess;
	}
}
}
