/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <QtDeclarative>
#include <util/gui/util.h>
#include <util/sys/paths.h>
#include <util/qml/widthiconprovider.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/ihavetabs.h>
#include "tablistview.h"
#include "launcherdroparea.h"
#include "tabunhidelistview.h"
#include "viewmanager.h"
#include "sbview.h"
#include "autoresizemixin.h"

Q_DECLARE_METATYPE (QSet<QByteArray>);

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
				TabClassName,
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
				roleNames [Roles::TabClassName] = "tabClassName";
				roleNames [Roles::OpenedTabsCount] = "openedTabsCount";
				roleNames [Roles::IsCurrentTab] = "isCurrentTab";
				roleNames [Roles::CanOpenTab] = "canOpenTab";
				roleNames [Roles::IsSingletonTab] = "isSingletonTab";
				setRoleNames (roleNames);
			}
		};
	}

	class TabClassImageProvider : public Util::WidthIconProvider
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

	LauncherComponent::LauncherComponent (ICoreTabWidget *ictw, ICoreProxy_ptr proxy, ViewManager *view, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, ICTW_ (ictw)
	, Model_ (new LauncherModel (this))
	, Component_ (new QuarkComponent ("sb2", "LauncherComponent.qml"))
	, View_ (view)
	, ImageProv_ (new TabClassImageProvider (proxy))
	{
		qmlRegisterType<LauncherDropArea> ("SB2", 1, 0, "LauncherDropArea");

		qRegisterMetaType<QSet<QByteArray>> ("QSet<QByteArray>");
		qRegisterMetaTypeStreamOperators<QSet<QByteArray>> ();

		Component_->DynamicProps_ << QPair<QString, QObject*> ("SB2_launcherModel", Model_);
		Component_->DynamicProps_ << QPair<QString, QObject*> ("SB2_launcherProxy", this);
		Component_->ImageProviders_ << QPair<QString, QDeclarativeImageProvider*> (ImageProviderID, ImageProv_);

		connect (ICTW_->GetQObject (),
				SIGNAL (currentChanged (int)),
				this,
				SLOT (handleCurrentTabChanged (int)));

		LoadHiddenTCs ();
	}

	QuarkComponent_ptr LauncherComponent::GetComponent () const
	{
		return Component_;
	}

	void LauncherComponent::SaveHiddenTCs () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("Launcher");
		settings.setValue ("HiddenTCs", QVariant::fromValue (HiddenTCs_));
		settings.endGroup ();
	}

	void LauncherComponent::LoadHiddenTCs ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_SB2");
		settings.beginGroup ("Launcher");
		HiddenTCs_ = settings.value ("HiddenTCs").value<decltype (HiddenTCs_)> ();
		FirstRun_ = settings.value ("FirstRun", true).toBool () && HiddenTCs_.isEmpty ();
		settings.setValue ("FirstRun", false);
		settings.endGroup ();
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
		if (!IsTabclassOpenable (tc) || HiddenTCs_.contains (tc.TabClass_))
			return 0;

		if (FirstRun_ && !(tc.Features_ & TabFeature::TFSuggestOpening))
		{
			HiddenTCs_ << tc.TabClass_;
			SaveHiddenTCs();
			return 0;
		}

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
		item->setData (tc.VisibleName_, LauncherModel::Roles::TabClassName);
		item->setData (0, LauncherModel::Roles::OpenedTabsCount);
		item->setData (false, LauncherModel::Roles::IsCurrentTab);
		item->setData (false, LauncherModel::Roles::CanOpenTab);
		Model_->appendRow (item);

		TC2Items_ [tc.TabClass_] << item;

		return item;
	}

	QPair<TabClassInfo, IHaveTabs*> LauncherComponent::FindTC (const QByteArray& tc) const
	{
		for (auto iht : Proxy_->GetPluginsManager ()->GetAllCastableTo<IHaveTabs*> ())
			for (const auto& fullTC : iht->GetTabClasses ())
				if (fullTC.TabClass_ == tc)
					return { fullTC, iht };

		return QPair<TabClassInfo, IHaveTabs*> ();
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
				TC2Obj_ [tc.TabClass_] = iht;
				TryAddTC (tc);
			}

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

		SaveHiddenTCs ();
	}

	void LauncherComponent::tabClassUnhideRequested (const QByteArray& tc)
	{
		if (!HiddenTCs_.remove (tc))
			return;

		SaveHiddenTCs ();

		if (!TC2Widgets_.value (tc).isEmpty ())
			return;

		const auto& pair = FindTC (tc);
		if (!pair.second)
		{
			qWarning () << Q_FUNC_INFO
					<< "tab class not found for"
					<< tc;
			return;
		}

		TryAddTC (pair.first);
		TC2Obj_ [tc] = pair.second;
	}

	void LauncherComponent::tabUnhideListRequested (int x, int y)
	{
		if (HiddenTCs_.isEmpty ())
			return;

		QList<TabClassInfo> tcs;
		for (const auto& tc : HiddenTCs_)
		{
			const auto& pair = FindTC (tc);
			if (pair.second)
				tcs << pair.first;
		}

		auto list = new TabUnhideListView (tcs, Proxy_);
		new AutoResizeMixin ({ x, y }, [this] () { return View_->GetFreeCoords (); }, list);
		list->show ();
		list->setFocus ();
		connect (list,
				SIGNAL (unhideRequested (QByteArray)),
				this,
				SLOT (tabClassUnhideRequested (QByteArray)));
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

		auto view = new TabListView (tc, widgets, ICTW_, Proxy_);
		view->move (Util::FitRect ({ x, y }, view->size (), View_->GetFreeCoords ()));
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
				ICTW_->Widget (idx) :
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
