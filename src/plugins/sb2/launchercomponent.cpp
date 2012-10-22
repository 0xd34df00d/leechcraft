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
#include <QtDebug>
#include <util/sys/paths.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ihavetabs.h>
#include "widthiconprovider.h"

namespace LeechCraft
{
namespace SB2
{
	namespace
	{
		class LauncherModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				TabClassIcon = Qt::UserRole + 1,
				TabClassID
			};

			LauncherModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::TabClassIcon] = "tabClassIcon";
				roleNames [Roles::TabClassID] = "tabClassID";
				setRoleNames (roleNames);
			}
		};
	}

	class TabClassImageProvider : public WidthIconProvider
	{
		ICoreProxy_ptr Proxy_;
		QHash<QByteArray, QIcon> TabClasses_;
	public:
		TabClassImageProvider (ICoreProxy_ptr proxy)
		: Proxy_ (proxy)
		{
		}

		QIcon GetIcon (const QStringList& list)
		{
			return TabClasses_.value (list.first ().toLatin1 ());
		}

		void AddTabClass (const TabClassInfo& tc)
		{
			TabClasses_ [tc.TabClass_] = tc.Icon_;
		}
	};

	namespace
	{
		const QString ImageProviderID = "SB2_TabClassImage";
	}

	LauncherComponent::LauncherComponent (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Model_ (new LauncherModel (this))
	, ImageProv_ (new TabClassImageProvider (proxy))
	{
		Component_.Url_ = Util::GetSysPath (Util::SysPath::QML, "sb2", "LauncherComponent.qml");
		Component_.DynamicProps_ << QPair<QString, QObject*> ("SB2_launcherModel", Model_);
		Component_.DynamicProps_ << QPair<QString, QObject*> ("SB2_launcherProxy", this);
		Component_.ImageProviders_ << QPair<QString, QDeclarativeImageProvider*> (ImageProviderID, ImageProv_);
	}

	QuarkComponent LauncherComponent::GetComponent () const
	{
		return Component_;
	}

	void LauncherComponent::handlePluginsAvailable ()
	{
		const auto& prefix = "image://" + ImageProviderID + '/';

		auto hasTabs = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveTabs*> ();
		for (auto ihtObj : hasTabs)
		{
			auto iht = qobject_cast<IHaveTabs*> (ihtObj);
			for (const auto& tc : iht->GetTabClasses ())
			{
				if (!(tc.Features_ & TabFeature::TFOpenableByRequest))
					continue;

				ImageProv_->AddTabClass (tc);

				auto item = new QStandardItem;
				item->setData (prefix + tc.TabClass_, LauncherModel::Roles::TabClassIcon);
				item->setData (tc.TabClass_, LauncherModel::Roles::TabClassID);
				Model_->appendRow (item);

				TC2Obj_ [tc.TabClass_] = iht;
			}
		}
	}

	void LauncherComponent::tabOpenRequested (const QByteArray& tc)
	{
		auto obj = TC2Obj_ [tc];
		if (!obj)
		{
			qWarning () << Q_FUNC_INFO
					<< "tabclass"
					<< tc
					<< "not found";
			return;
		}

		obj->TabOpenRequested (tc);
	}
}
}
