/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fsdisplayer.h"
#include <algorithm>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QApplication>
#include <util/util.h>
#include <util/gui/geometry.h>
#include <util/qml/themeimageprovider.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/util.h>
#include <util/sys/paths.h>
#include <util/xdg/itemsfinder.h>
#include <util/xdg/item.h>
#include <util/models/rolenamesmixin.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/ihavetabs.h>
#include "itemssortfilterproxymodel.h"
#include "modelroles.h"
#include "favoritesmanager.h"
#include "recentmanager.h"
#include "syspathitemprovider.h"

namespace LC
{
namespace Launchy
{
	class ItemIconsProvider : public QQuickImageProvider
	{
		ICoreProxy_ptr Proxy_;
		QHash<QString, QIcon> Icons_;
	public:
		explicit ItemIconsProvider (const ICoreProxy_ptr& proxy)
		: QQuickImageProvider (Pixmap)
		, Proxy_ (proxy)
		{
		}

		void Clear ()
		{
			Icons_.clear ();
		}

		void AddIcon (const QString& id, const QIcon& icon)
		{
			Icons_ [id] = icon;
		}

		QPixmap requestPixmap (const QString& id, QSize* size, const QSize& requestedSize) override
		{
			auto icon = Icons_.value (id);
			if (icon.isNull ())
				icon = Proxy_->GetIconThemeManager ()->GetIcon ("system-run");

			const auto& ourSize = requestedSize.width () > 1 ?
					requestedSize :
					QSize (96, 96);

			if (size)
				*size = icon.actualSize (ourSize);
			return icon.pixmap (ourSize);
		}
	};

	namespace
	{
		class DisplayModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			explicit DisplayModel (QObject *parent = nullptr)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [ModelRoles::CategoryName] = "categoryName";
				roleNames [ModelRoles::CategoryIcon] = "categoryIcon";
				roleNames [ModelRoles::CategoryType] = "categoryType";
				roleNames [ModelRoles::ItemName] = "itemName";
				roleNames [ModelRoles::ItemIcon] = "itemIcon";
				roleNames [ModelRoles::ItemDescription] = "itemDescription";
				roleNames [ModelRoles::ItemID] = "itemID";
				roleNames [ModelRoles::IsItemFavorite] = "isItemFavorite";
				setRoleNames (roleNames);
			}
		};
	}

	FSDisplayer::FSDisplayer (ICoreProxy_ptr proxy, Util::XDG::ItemsFinder *finder,
			FavoritesManager *favMgr, RecentManager *recMgr, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Finder_ (finder)
	, FavManager_ (favMgr)
	, RecentManager_ (recMgr)
	, CatsModel_ (new DisplayModel (this))
	, ItemsModel_ (new DisplayModel (this))
	, ItemsProxyModel_ (new ItemsSortFilterProxyModel (ItemsModel_, this))
	, View_ (std::make_shared<QQuickWidget> ())
	, IconsProvider_ (new ItemIconsProvider (proxy))
	, SysPathHandler_ (new SysPathItemProvider (ItemsModel_, this))
	{
		Util::SetupFullscreenView (*View_);
		Util::EnableTransparency (*View_);
		Util::WatchQmlErrors (*View_);

		View_->engine ()->addImageProvider ("appicon", IconsProvider_);
		View_->engine ()->addImageProvider ("theme", new Util::ThemeImageProvider (proxy));
		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			View_->engine ()->addImportPath (cand);

		View_->rootContext ()->setContextProperty ("itemsModel", ItemsProxyModel_);
		View_->rootContext ()->setContextProperty ("catsModel", CatsModel_);
		View_->rootContext ()->setContextProperty ("launchyProxy", this);
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (proxy->GetColorThemeManager (), parent));

		View_->setSource (Util::GetSysPathUrl (Util::SysPath::QML, "launchy", "FSView.qml"));

		connect (View_->rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));
		connect (View_->rootObject (),
				SIGNAL (categorySelected (int)),
				this,
				SLOT (handleCategorySelected (int)));
		connect (View_->rootObject (),
				SIGNAL (itemSelected (QString)),
				this,
				SLOT (handleExecRequested (QString)));
		connect (View_->rootObject (),
				SIGNAL (itemBookmarkRequested (QString)),
				this,
				SLOT (handleItemBookmark (QString)));

		handleFinderUpdated ();
		handleCategorySelected (0);
	}

	QString FSDisplayer::GetAppFilterText () const
	{
		return ItemsProxyModel_->GetAppFilterText ();
	}

	void FSDisplayer::SetAppFilterText (const QString& text)
	{
		ItemsProxyModel_->SetAppFilterText (text);
		emit appFilterTextChanged ();

		SysPathHandler_->HandleQuery (text);
	}

	void FSDisplayer::MakeStdCategories ()
	{
		auto addCustomCat = [this] (const QString& name, const QString& native,
				const QString& iconName, const QIcon& icon) -> void
		{
			auto cat = new QStandardItem;
			cat->setData (name, ModelRoles::CategoryName);
			cat->setData (QStringList (native), ModelRoles::NativeCategories);
			cat->setData (iconName, ModelRoles::CategoryIcon);
			cat->setData ("std", ModelRoles::CategoryType);
			if (!icon.isNull ())
				IconsProvider_->AddIcon (iconName, icon);

			CatsModel_->appendRow (cat);
		};

		if (RecentManager_->HasRecents ())
			addCustomCat (tr ("Recent"), "X-Recent", "document-open-recent",
					Proxy_->GetIconThemeManager ()->GetIcon ("document-open-recent"));
		addCustomCat ("LeechCraft", "X-LeechCraft", "leechcraft",
				QIcon ("lcicons:/resources/images/leechcraft.svg"));
		addCustomCat (tr ("Favorites"), "X-Favorites", "favorites",
				Proxy_->GetIconThemeManager ()->GetIcon ("favorites"));
	}

	void FSDisplayer::MakeStdItems ()
	{
		const auto& tabs = Proxy_->GetPluginsManager ()->GetAllCastableTo<IHaveTabs*> ();
		for (IHaveTabs *iht : tabs)
			for (const auto& tc : iht->GetTabClasses ())
			{
				if (!(tc.Features_ & TabFeature::TFOpenableByRequest))
					continue;

				const auto& iconId = tc.TabClass_;
				IconsProvider_->AddIcon (iconId, tc.Icon_);

				auto item = new QStandardItem;
				item->setData (tc.VisibleName_, ModelRoles::ItemName);
				item->setData (tc.Description_, ModelRoles::ItemDescription);
				item->setData (iconId, ModelRoles::ItemIcon);
				item->setData (QStringList ("X-LeechCraft"), ModelRoles::ItemNativeCategories);
				item->setData (tc.TabClass_, ModelRoles::ItemID);
				item->setData (FavManager_->IsFavorite (tc.TabClass_), ModelRoles::IsItemFavorite);
				item->setData (false, ModelRoles::IsItemRecent);

				auto executor = [iht, tc] { iht->TabOpenRequested (tc.TabClass_); };
				item->setData (QVariant::fromValue<Executor_f> (executor),
						ModelRoles::ExecutorFunctor);

				ItemsModel_->appendRow (item);
			}
	}

	namespace
	{
		struct CategoriesInfo
		{
			struct SingleInfo
			{
				QString TranslatedName_;
				QString IconName_;
			};
			QHash<QString, SingleInfo> Infos_;

			CategoriesInfo ()
			{
				Infos_ ["Accessories"] = { FSDisplayer::tr ("Accessories"), "applications-accessories" };
				Infos_ ["Development"] = { FSDisplayer::tr ("Development"), "applications-development" };
				Infos_ ["Education"] = { FSDisplayer::tr ("Education"), "applications-education" };
				Infos_ ["Games"] = { FSDisplayer::tr ("Games"), "applications-games" };
				Infos_ ["Graphics"] = { FSDisplayer::tr ("Graphics"), "applications-graphics" };
				Infos_ ["Internet"] = { FSDisplayer::tr ("Internet"), "applications-internet" };
				Infos_ ["Multimedia"] = { FSDisplayer::tr ("Multimedia"), "applications-multimedia" };
				Infos_ ["Office"] = { FSDisplayer::tr ("Office"), "applications-office" };
				Infos_ ["Other"] = { FSDisplayer::tr ("Other"), "applications-other" };
				Infos_ ["Settings"] = { FSDisplayer::tr ("Settings"), "preferences-system" };
				Infos_ ["Science"] = { FSDisplayer::tr ("Science"), "applications-science" };
				Infos_ ["System"] = { FSDisplayer::tr ("System"), "applications-system" };
				Infos_ ["Toys"] = { FSDisplayer::tr ("Toys"), "applications-toys" };
				Infos_ ["Utilities"] = { FSDisplayer::tr ("Utilities"), "applications-utilities" };

				Infos_ ["Debugger"] = Infos_ ["Development"];
				Infos_ ["IDE"] = Infos_ ["Development"];
				Infos_ ["Translation"] = Infos_ ["Development"];

				Infos_ ["InstantMessaging"] = Infos_ ["Internet"];
				Infos_ ["Network"] = Infos_ ["Internet"];
				Infos_ ["P2P"] = Infos_ ["Internet"];
				Infos_ ["WebBrowser"] = Infos_ ["Internet"];

				Infos_ ["RasterGraphics"] = Infos_ ["Graphics"];

				Infos_ ["Audio"] = Infos_ ["Multimedia"];
				Infos_ ["AudioVideo"] = Infos_ ["Multimedia"];
				Infos_ ["Music"] = Infos_ ["Multimedia"];
				Infos_ ["Player"] = Infos_ ["Multimedia"];
				Infos_ ["Recorder"] = Infos_ ["Multimedia"];

				Infos_ ["DesktopSettings"] = Infos_ ["Settings"];
				Infos_ ["HardwareSettings"] = Infos_ ["Settings"];

				Infos_ ["Application"] = Infos_ ["Utilities"];
				Infos_ ["Utility"] = Infos_ ["Utilities"];
			}
		};
	}

	void FSDisplayer::MakeCategories (const QStringList& cats)
	{
		MakeStdCategories ();

		static const CategoriesInfo cInfo;

		QMap<QString, QStandardItem*> catItems;
		for (const auto& cat : cats)
		{
			if (!cInfo.Infos_.contains (cat))
			{
				qDebug () << Q_FUNC_INFO << "skipping" << cat;
				continue;
			}

			const auto& catInfo = cInfo.Infos_ [cat];
			const auto& visibleName = catInfo.TranslatedName_;
			if (catItems.contains (visibleName))
			{
				auto item = catItems [visibleName];
				const auto& list = item->data (ModelRoles::NativeCategories).toStringList ();
				item->setData (list + QStringList (cat), ModelRoles::NativeCategories);
				continue;
			}

			if (!catInfo.IconName_.isEmpty ())
				IconsProvider_->AddIcon (catInfo.IconName_,
						Proxy_->GetIconThemeManager ()->GetIcon (catInfo.IconName_));

			auto catItem = new QStandardItem;
			catItem->setData (visibleName, ModelRoles::CategoryName);
			catItem->setData (QStringList (cat), ModelRoles::NativeCategories);
			catItem->setData (catInfo.IconName_, ModelRoles::CategoryIcon);
			catItem->setData ("xdg", ModelRoles::CategoryType);
			catItems [visibleName] = catItem;
		}
		for (auto item : catItems.values ())
			CatsModel_->appendRow (item);
	}

	void FSDisplayer::MakeItems (const QList<QList<Util::XDG::Item_ptr>>& items)
	{
		MakeStdItems ();

		const auto& curLang = Util::GetLanguage ().toLower ();

		QList<Util::XDG::Item_ptr> uniqueItems;
		for (const auto& sublist : items)
			for (const auto& item : sublist)
				if (!item->IsHidden () &&
						std::none_of (uniqueItems.begin (), uniqueItems.end (),
								[&item] (const auto& other) { return *other == *item; }))
					uniqueItems << item;

		std::sort (uniqueItems.begin (), uniqueItems.end (),
				[&curLang] (const auto& left, const auto& right)
				{
					return QString::localeAwareCompare (left->GetName (curLang), right->GetName (curLang)) < 0;
				});
		for (const auto& item : uniqueItems)
		{
			const auto& itemName = item->GetName (curLang);

			auto appItem = new QStandardItem ();
			appItem->setData (itemName, ModelRoles::ItemName);

			appItem->setData (item->GetCommand (), ModelRoles::ItemCommand);

			auto comment = item->GetComment (curLang);
			if (comment.isEmpty ())
				comment = item->GetGenericName (curLang);
			appItem->setData (comment, ModelRoles::ItemDescription);

			const auto& iconName = item->GetIconName ();
			appItem->setData (iconName, ModelRoles::ItemIcon);
			IconsProvider_->AddIcon (iconName, item->GetIcon (Proxy_));

			appItem->setData (item->GetCategories (), ModelRoles::ItemNativeCategories);
			appItem->setData (item->GetPermanentID (), ModelRoles::ItemID);
			appItem->setData (FavManager_->IsFavorite (item->GetPermanentID ()),
					ModelRoles::IsItemFavorite);

			const auto isRecent = RecentManager_->IsRecent (item->GetPermanentID ());
			appItem->setData (isRecent,
					ModelRoles::IsItemRecent);
			if (isRecent)
				appItem->setData (RecentManager_->GetRecentOrder (item->GetPermanentID ()),
						ModelRoles::ItemRecentPos);

			auto executor = [this, item] { item->Execute (Proxy_); };
			appItem->setData (QVariant::fromValue<Executor_f> (executor),
					ModelRoles::ExecutorFunctor);

			ItemsModel_->appendRow (appItem);
		}
	}

	QStandardItem* FSDisplayer::FindItem (const QString& itemId) const
	{
		for (int i = 0, rc = ItemsModel_->rowCount (); i < rc; ++i)
		{
			auto item = ItemsModel_->item (i);
			if (item->data (ModelRoles::ItemID) == itemId)
				return item;
		}

		return 0;
	}

	void FSDisplayer::handleFinderUpdated ()
	{
		CatsModel_->clear ();
		ItemsModel_->clear ();
		ItemsProxyModel_->setCategoryNames (QStringList ());

		IconsProvider_->Clear ();

		const auto& categorizedItems = Finder_->GetItems ();
		MakeCategories (categorizedItems.keys ());
		MakeItems (categorizedItems.values ());

		View_->showFullScreen ();

		View_->setFocus ();
		View_->setFocus (Qt::MouseFocusReason);
	}

	void FSDisplayer::handleCategorySelected (int row)
	{
		auto item = CatsModel_->item (row);
		const auto& list = item->data (ModelRoles::NativeCategories).toStringList ();
		ItemsProxyModel_->setCategoryNames (list);
	}

	void FSDisplayer::handleExecRequested (const QString& id)
	{
		auto item = FindItem (id);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no such item"
					<< id;
			return;
		}

		RecentManager_->AddRecent (id);
		item->data (ModelRoles::ExecutorFunctor).value<Executor_f> () ();

		deleteLater ();
	}

	void FSDisplayer::handleItemBookmark (const QString& id)
	{
		auto item = FindItem (id);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "no such item"
					<< id;
			return;
		}

		FavManager_->AddFavorite (item->data (ModelRoles::ItemID).toString ());
		item->setData (true, ModelRoles::IsItemFavorite);
	}
}
}
