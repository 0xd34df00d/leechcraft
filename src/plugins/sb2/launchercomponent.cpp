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

#include "launchercomponent.h"
#include <QStandardItemModel>
#include <util/sys/paths.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ihavetabs.h>

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		class LauncherModel : public QStandardItemModel
		{
		public:
			LauncherModel (QObject *parent)
			: QStandardItemModel (parent)
			{
			}
		};
	}

	LauncherComponent::LauncherComponent (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Model_ (new LauncherModel (this))
	{
		Component_.Url_ = Util::GetSysPath (Util::SysPath::QML, "sb2", "LauncherComponent.qml");
		Component_.DynamicProps_ << QPair<QString, QObject*> ("SB2_launcherModel", Model_);
		//Component_.ImageProviders_ << QPair<QString, QDeclarativeImageProvider*> (ImageProviderID, ImageProv_);
	}

	QuarkComponent LauncherComponent::GetComponent () const
	{
		return Component_;
	}

	void LauncherComponent::handlePluginsAvailable ()
	{
		auto hasTabs = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveTabs*> ();
		for (auto ihtObj : hasTabs)
		{
			auto iht = qobject_cast<IHaveTabs*> (ihtObj);
			for (const auto& tc : iht->GetTabClasses ())
			{
				if (!(tc.Features_ & TabFeature::TFOpenableByRequest))
					continue;

				auto item = new QStandardItem;
			}
		}
	}
}
}
