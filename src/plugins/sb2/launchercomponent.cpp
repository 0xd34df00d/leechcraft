/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "launchercomponent.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QtDebug>
#include <util/sll/scopeguards.h>
#include <util/sll/qtutil.h>
#include <util/gui/autoresizemixin.h>
#include <util/gui/geometry.h>
#include <util/qml/widthiconprovider.h>
#include <util/models/rolenamesmixin.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/icoretabwidget.h>
#include <interfaces/ihavetabs.h>
#include "tablistview.h"
#include "launcherdroparea.h"
#include "tabunhidelistview.h"
#include "viewmanager.h"
#include "sbview.h"

Q_DECLARE_METATYPE (QSet<QByteArray>);

namespace LC::SB2
{
	namespace
	{
		class LauncherModel : public Util::RoleNamesMixin<QStandardItemModel>
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

			explicit LauncherModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::TabClassIcon] = QByteArrayLiteral ("tabClassIcon");
				roleNames [Roles::TabClassID] = QByteArrayLiteral ("tabClassID");
				roleNames [Roles::TabClassName] = QByteArrayLiteral ("tabClassName");
				roleNames [Roles::OpenedTabsCount] = QByteArrayLiteral ("openedTabsCount");
				roleNames [Roles::IsCurrentTab] = QByteArrayLiteral ("isCurrentTab");
				roleNames [Roles::CanOpenTab] = QByteArrayLiteral ("canOpenTab");
				roleNames [Roles::IsSingletonTab] = QByteArrayLiteral ("isSingletonTab");
				setRoleNames (roleNames);
			}
		};
	}

	class TabClassImageProvider final : public Util::WidthIconProvider
	{
		QHash<QByteArray, QIcon> TabClasses_;
	public:
		TabClassImageProvider () = default;
		TabClassImageProvider (const TabClassImageProvider&) = delete;
		TabClassImageProvider (TabClassImageProvider&&) = delete;

		QIcon GetIcon (const QStringList& list)
		{
			return TabClasses_.value (list.join ('/').toLatin1 ());
		}

		void AddTabClass (const TabClassInfo& tc)
		{
			TabClasses_ [tc.TabClass_] = tc.Icon_;
		}
	};

	namespace
	{
		const QString ImageProviderID = QStringLiteral ("SB2_TabClassImage");
	}

	LauncherComponent::LauncherComponent (ICoreTabWidget *ictw, ViewManager *view, QObject *parent)
	: QObject (parent)
	, ICTW_ (ictw)
	, Model_ (new LauncherModel (this))
	, Component_ (new QuarkComponent (QStringLiteral ("sb2"), QStringLiteral ("LauncherComponent.qml")))
	, View_ (view)
	, ImageProv_ (new TabClassImageProvider ())
	{
		qmlRegisterType<LauncherDropArea> ("SB2", 1, 0, "LauncherDropArea");

		Component_->DynamicProps_.append ({ QStringLiteral ("SB2_launcherModel"), Model_ });
		Component_->DynamicProps_.append ({ QStringLiteral ("SB2_launcherProxy"), this });
		Component_->ImageProviders_.append ({ ImageProviderID, ImageProv_ });

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
		auto settings = View_->GetSettings ();
		auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("Launcher"));
		settings->setValue (QStringLiteral ("HiddenTCs"), QVariant::fromValue (HiddenTCs_));
	}

	void LauncherComponent::LoadHiddenTCs ()
	{
		auto settings = View_->GetSettings ();
		auto groupGuard = Util::BeginGroup (*settings, QStringLiteral ("Launcher"));
		HiddenTCs_ = settings->value (QStringLiteral ("HiddenTCs")).value<decltype (HiddenTCs_)> ();
		FirstRun_ = settings->value (QStringLiteral ("FirstRun"), true).toBool () && HiddenTCs_.isEmpty ();
		settings->setValue (QStringLiteral ("FirstRun"), false);
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
			return nullptr;

		if (FirstRun_ && !(tc.Features_ & TabFeature::TFSuggestOpening))
		{
			HiddenTCs_ << tc.TabClass_;
			SaveHiddenTCs();
			return nullptr;
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
		for (auto iht : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IHaveTabs*> ())
			for (const auto& fullTC : iht->GetTabClasses ())
				if (fullTC.TabClass_ == tc)
					return { fullTC, iht };

		return QPair<TabClassInfo, IHaveTabs*> ();
	}

	void LauncherComponent::handlePluginsAvailable ()
	{
		auto hasTabs = GetProxyHolder ()->GetPluginsManager ()->
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

		auto list = new TabUnhideListView (tcs);
		new Util::AutoResizeMixin ({ x, y }, [this] () { return View_->GetFreeCoords (); }, list);
		list->show ();
		list->setFocus ();
		connect (list,
				&TabUnhideListView::unhideRequested,
				this,
				&LauncherComponent::tabClassUnhideRequested);
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
		{
			CurrentTabList_->deleteLater ();
			CurrentTabList_ = nullptr;
		}

		auto view = new TabListView (tc, widgets, ICTW_, View_->GetManagedWindow ());
		view->show ();
		const auto& pos = Util::FitRect ({ x, y }, view->size (), View_->GetFreeCoords ());
		view->move (pos);
		view->setFocus ();

		CurrentTabList_ = view;
	}

	void LauncherComponent::tabListUnhovered (const QByteArray& tc)
	{
		if (!CurrentTabList_)
			return;

		if (tc == CurrentTabList_->GetTabClass ())
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
				nullptr;

		const auto& tc = widget ?
				qobject_cast<ITabWidget*> (widget)->GetTabClassInfo () :
				TabClassInfo ();

		for (const auto& [key, items] : Util::Stlize (TC2Items_))
		{
			const bool isSelectedTC = key == tc.TabClass_;
			for (auto item : items)
				item->setData (isSelectedTC, LauncherModel::Roles::IsCurrentTab);
		}
	}
}
