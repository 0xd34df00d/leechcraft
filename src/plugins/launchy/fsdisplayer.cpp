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

#include "fsdisplayer.h"
#include <algorithm>
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeImageProvider>
#include <QGraphicsObject>
#include <QStandardItemModel>
#include <QApplication>
#include <QDesktopWidget>
#include <QProcess>
#include <util/util.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/ihavetabs.h>
#include "itemsfinder.h"
#include "item.h"
#include "itemssortfilterproxymodel.h"
#include "modelroles.h"

namespace LeechCraft
{
namespace Launchy
{
	class ItemIconsProvider : public QDeclarativeImageProvider
	{
		ICoreProxy_ptr Proxy_;
		QHash<QString, QIcon> Icons_;
	public:
		ItemIconsProvider (ICoreProxy_ptr proxy)
		: QDeclarativeImageProvider (Pixmap)
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

		QPixmap requestPixmap (const QString& id, QSize* size, const QSize& requestedSize)
		{
			auto icon = Icons_.value (id);
			if (icon.isNull ())
				icon = Proxy_->GetIcon ("system-run");

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
		class DisplayModel : public QStandardItemModel
		{
		public:
			DisplayModel (QObject *parent = 0)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [ModelRoles::CategoryName] = "categoryName";
				roleNames [ModelRoles::CategoryIcon] = "categoryIcon";
				roleNames [ModelRoles::ItemName] = "itemName";
				roleNames [ModelRoles::ItemIcon] = "itemIcon";
				roleNames [ModelRoles::ItemDescription] = "itemDescription";
				roleNames [ModelRoles::ItemID] = "itemID";
				setRoleNames (roleNames);
			}
		};
	}

	FSDisplayer::FSDisplayer (ICoreProxy_ptr proxy, ItemsFinder *finder, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Finder_ (finder)
	, CatsModel_ (new DisplayModel (this))
	, ItemsModel_ (new DisplayModel (this))
	, ItemsProxyModel_ (new ItemsSortFilterProxyModel (ItemsModel_, this))
	, View_ (new QDeclarativeView)
	, IconsProvider_ (new ItemIconsProvider (proxy))
	{
		View_->setStyleSheet ("background: transparent");
		View_->setWindowFlags (Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
		View_->setAttribute (Qt::WA_TranslucentBackground);
		View_->setAttribute (Qt::WA_OpaquePaintEvent, false);

		const auto& rect = qApp->desktop ()->screenGeometry (QCursor::pos ());
		View_->setGeometry (rect);
		View_->setFixedSize (rect.size ());

		View_->engine ()->addImageProvider ("appicon", IconsProvider_);

		View_->setResizeMode (QDeclarativeView::SizeRootObjectToView);
		View_->rootContext ()->setContextProperty ("itemsModel", ItemsProxyModel_);
		View_->rootContext ()->setContextProperty ("itemsModelFilter", ItemsProxyModel_);
		View_->rootContext ()->setContextProperty ("catsModel", CatsModel_);

		connect (View_,
				SIGNAL (statusChanged (QDeclarativeView::Status)),
				this,
				SLOT (handleViewStatus (QDeclarativeView::Status)));
		View_->setSource (QUrl ("qrc:/launchy/resources/qml/FSView.qml"));

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

		handleFinderUpdated ();
		handleCategorySelected (0);
	}

	FSDisplayer::~FSDisplayer ()
	{
		delete View_;
	}

	void FSDisplayer::Execute (Item_ptr item)
	{
		auto command = item->GetCommand ();

		if (item->GetType () == Item::Type::Application)
		{
			command.remove ("%c");
			command.remove ("%f");
			command.remove ("%F");
			command.remove ("%u");
			command.remove ("%U");
			command.remove ("%i");
			auto items = command.split (' ', QString::SkipEmptyParts);
			auto removePred = [] (const QString& str)
				{ return str.size () == 2 && str.at (0) == '%'; };
			items.erase (std::remove_if (items.begin (), items.end (), removePred),
					items.end ());
			if (items.isEmpty ())
				return;

			QProcess::startDetached (items.at (0), items.mid (1), item->GetWorkingDirectory ());
		}
		else if (item->GetType () == Item::Type::URL)
		{
			const auto& e = Util::MakeEntity (QUrl (command),
					QString (),
					FromUserInitiated | OnlyHandle);
			emit gotEntity (e);
		}
		else
		{
			qWarning () << Q_FUNC_INFO
					<< "don't know how to execute this type of app";
		}
	}

	void FSDisplayer::MakeStdCategories ()
	{
		auto lcCat = new QStandardItem;
		lcCat->setData ("LeechCraft", ModelRoles::CategoryName);
		lcCat->setData (QStringList ("X-LeechCraft"), ModelRoles::NativeCategories);
		lcCat->setData ("leechcraft", ModelRoles::CategoryIcon);

		IconsProvider_->AddIcon ("leechcraft", QIcon (":/resources/images/leechcraft.svg"));
		CatsModel_->appendRow (lcCat);
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

				Execs_ [tc.TabClass_] = [iht, tc] () { iht->TabOpenRequested (tc.TabClass_); };

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

				Infos_ ["Application"] = Infos_ ["Utilities"];
				Infos_ ["InstantMessaging"] = Infos_ ["Internet"];
				Infos_ ["Music"] = Infos_ ["Multimedia"];
				Infos_ ["Network"] = Infos_ ["Internet"];
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
				IconsProvider_->AddIcon (catInfo.IconName_, Proxy_->GetIcon (catInfo.IconName_));

			auto catItem = new QStandardItem;
			catItem->setData (visibleName, ModelRoles::CategoryName);
			catItem->setData (QStringList (cat), ModelRoles::NativeCategories);
			catItem->setData (catInfo.IconName_, ModelRoles::CategoryIcon);
			catItems [visibleName] = catItem;
		}
		for (auto item : catItems.values ())
			CatsModel_->appendRow (item);
	}

	void FSDisplayer::MakeItems (const QList<QList<Item_ptr>>& items)
	{
		MakeStdItems ();

		const auto& curLang = Util::GetLanguage ().toLower ();

		QList<Item_ptr> uniqueItems;
		for (const auto& sublist : items)
			for (auto item : sublist)
				if (!item->IsHidden () &&
					std::find_if (uniqueItems.begin (), uniqueItems.end (),
							[&item] (const Item_ptr other) { return *other == *item; }) == uniqueItems.end ())
					uniqueItems << item;
		std::sort (uniqueItems.begin (), uniqueItems.end (),
				[&curLang] (Item_ptr left, Item_ptr right)
				{
					return QString::localeAwareCompare (left->GetName (curLang), right->GetName (curLang)) < 0;
				});
		for (const auto& item : uniqueItems)
		{
			const auto& itemName = item->GetName (curLang);

			auto appItem = new QStandardItem ();
			appItem->setData (itemName, ModelRoles::ItemName);

			auto comment = item->GetComment (curLang);
			if (comment.isEmpty ())
				comment = item->GetGenericName (curLang);
			appItem->setData (comment, ModelRoles::ItemDescription);

			const auto& iconName = item->GetIconName ();
			appItem->setData (iconName, ModelRoles::ItemIcon);
			IconsProvider_->AddIcon (iconName, item->GetIcon ());

			appItem->setData (item->GetCategories (), ModelRoles::ItemNativeCategories);

			appItem->setData (itemName, ModelRoles::ItemID);
			Execs_ [itemName] = [this, item] () { Execute (item); };

			ItemsModel_->appendRow (appItem);
		}
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

	void FSDisplayer::handleExecRequested (const QString& item)
	{
		if (!Execs_.contains (item))
		{
			qWarning () << Q_FUNC_INFO
					<< "no such item"
					<< item;
			return;
		}

		Execs_ [item] ();
		deleteLater ();
	}

	void FSDisplayer::handleViewStatus (QDeclarativeView::Status status)
	{
		if (status != QDeclarativeView::Error)
			return;

		qWarning () << Q_FUNC_INFO << "declarative view error";
		deleteLater ();
	}
}
}
