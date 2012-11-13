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
#include <util/util.h>
#include <util/sys/paths.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/ihavetabs.h>
#include "widthiconprovider.h"
#include "tablistview.h"

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
				TabClassID,
				OpenedTabsCount,
				IsCurrentTab,
				CanOpenTab,
				IsSingletonTab
			};

			LauncherModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::TabClassIcon] = "tabClassIcon";
				roleNames [Roles::TabClassID] = "tabClassID";
				roleNames [Roles::OpenedTabsCount] = "openedTabsCount";
				roleNames [Roles::IsCurrentTab] = "isCurrentTab";
				roleNames [Roles::CanOpenTab] = "canOpenTab";
				roleNames [Roles::IsSingletonTab] = "isSingletonTab";
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
		Component_.Url_ = QUrl::fromLocalFile (Util::GetSysPath (Util::SysPath::QML, "sb2", "LauncherComponent.qml"));
		Component_.DynamicProps_ << QPair<QString, QObject*> ("SB2_launcherModel", Model_);
		Component_.DynamicProps_ << QPair<QString, QObject*> ("SB2_launcherProxy", this);
		Component_.ImageProviders_ << QPair<QString, QDeclarativeImageProvider*> (ImageProviderID, ImageProv_);

		connect (proxy->GetTabWidget ()->GetObject (),
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleCurrentTabChanged (int)));
	}

	QuarkComponent LauncherComponent::GetComponent () const
	{
		return Component_;
	}

	namespace
	{
		bool IsTabclassOpenable (const TabClassInfo& tc)
		{
			if (!(tc.Features_ & TabFeature::TFOpenableByRequest))
				return false;

			if (tc.Icon_.isNull ())
				return false;

			if (!tc.Priority_)
				return false;

			return true;
		}
	}

	QStandardItem* LauncherComponent::TryAddTC (const TabClassInfo& tc)
	{
		if (!IsTabclassOpenable (tc))
			return 0;

		auto item = CreateItem (tc);
		item->setData (true, LauncherModel::Roles::CanOpenTab);
		const bool isSingle = tc.Features_ & TabFeature::TFSingle;
		item->setData (isSingle, LauncherModel::Roles::IsSingletonTab);
		return item;
	}

	QStandardItem* LauncherComponent::CreateItem (const TabClassInfo& tc)
	{
		const auto& prefix = "image://" + ImageProviderID + '/';

		ImageProv_->AddTabClass (tc);

		auto item = new QStandardItem;
		item->setData (prefix + tc.TabClass_, LauncherModel::Roles::TabClassIcon);
		item->setData (tc.TabClass_, LauncherModel::Roles::TabClassID);
		item->setData (0, LauncherModel::Roles::OpenedTabsCount);
		item->setData (false, LauncherModel::Roles::IsCurrentTab);
		Model_->appendRow (item);

		TC2Items_ [tc.TabClass_] << item;

		return item;
	}

	void LauncherComponent::handlePluginsAvailable ()
	{
		auto hasTabs = Proxy_->GetPluginsManager ()->
				GetAllCastableRoots<IHaveTabs*> ();
		for (auto ihtObj : hasTabs)
		{
			auto iht = qobject_cast<IHaveTabs*> (ihtObj);
			for (const auto& tc : iht->GetTabClasses ())
				if (TryAddTC (tc))
					TC2Obj_ [tc.TabClass_] = iht;

			connect (ihtObj,
					SIGNAL (addNewTab (QString, QWidget*)),
					this,
					SLOT (handleNewTab (QString, QWidget*)));
			connect (ihtObj,
					SIGNAL (removeTab (QWidget*)),
					this,
					SLOT (handleRemoveTab (QWidget*)));
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

	void LauncherComponent::tabClassHideRequested (const QByteArray& tc)
	{
		if (HiddenTCs_.contains (tc))
			return;

		HiddenTCs_ << tc;
		if (TC2Widgets_.value (tc).isEmpty ())
			for (auto item : TC2Items_.take (tc))
				Model_->removeRow (item->row ());
	}

	void LauncherComponent::tabListRequested (const QByteArray& tc, int x, int y)
	{
		const auto& widgets = TC2Widgets_ [tc];
		if (widgets.isEmpty ())
			return;

		if (CurrentTabList_ && CurrentTabList_->GetTabClass () == tc)
		{
			CurrentTabList_->HandleLauncherHovered ();
			return;
		}

		if (CurrentTabList_)
			delete CurrentTabList_;

		auto view = new TabListView (tc, widgets, Proxy_);
		view->move (x, y);
		view->show ();
		view->setFocus ();

		CurrentTabList_ = view;
	}

	void LauncherComponent::tabListUnhovered (const QByteArray&)
	{
		if (!CurrentTabList_)
			return;

		CurrentTabList_->HandleLauncherUnhovered ();
	}

	void LauncherComponent::handleNewTab (const QString&, QWidget *w)
	{
		auto itw = qobject_cast<ITabWidget*> (w);
		const auto& tc = itw->GetTabClassInfo ();

		auto& wList = TC2Widgets_ [tc.TabClass_];
		if (wList.contains (w))
			return;

		wList << w;

		if (!TC2Items_.contains (tc.TabClass_))
			CreateItem (tc);

		for (auto item : TC2Items_ [tc.TabClass_])
			item->setData (wList.size (), LauncherModel::Roles::OpenedTabsCount);
	}

	void LauncherComponent::handleRemoveTab (QWidget *w)
	{
		auto itw = qobject_cast<ITabWidget*> (w);
		const auto& tc = itw->GetTabClassInfo ();

		auto& wList = TC2Widgets_ [tc.TabClass_];
		wList.removeAll (w);

		if (wList.isEmpty () && (!IsTabclassOpenable (tc) || HiddenTCs_.contains (tc.TabClass_)))
			for (auto item : TC2Items_.take (tc.TabClass_))
				Model_->removeRow (item->row ());
		else
			for (auto item : TC2Items_ [tc.TabClass_])
				item->setData (wList.size (), LauncherModel::Roles::OpenedTabsCount);
	}

	void LauncherComponent::handleCurrentTabChanged (int idx)
	{
		auto widget = idx >= 0 ?
				Proxy_->GetTabWidget ()->Widget (idx) :
				0;

		const auto& tc = widget ?
				qobject_cast<ITabWidget*> (widget)->GetTabClassInfo () :
				TabClassInfo ();

		for (const auto& key : TC2Items_.keys ())
		{
			const bool isSelectedTC = key == tc.TabClass_;
			for (auto item : TC2Items_ [key])
				item->setData (isSelectedTC, LauncherModel::Roles::IsCurrentTab);
		}
	}
}
}
