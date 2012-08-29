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
#include <QDeclarativeView>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDeclarativeImageProvider>
#include <QGraphicsObject>
#include <QStandardItemModel>
#include <QApplication>
#include <QDesktopWidget>
#include <util/util.h>
#include "itemsfinder.h"
#include "item.h"

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
			enum Roles
			{
				CategoryName = Qt::UserRole + 1,
				CategoryIcon,
				ItemName,
				ItemIcon,
				ItemDescription
			};

			DisplayModel (QObject *parent = 0)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [CategoryName] = "categoryName";
				roleNames [CategoryIcon] = "categoryIcon";
				roleNames [ItemName] = "itemName";
				roleNames [ItemIcon] = "itemIcon";
				roleNames [ItemDescription] = "itemDescription";
				setRoleNames (roleNames);
			}
		};
	}

	FSDisplayer::FSDisplayer (ICoreProxy_ptr proxy, ItemsFinder *finder, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, Finder_ (finder)
	, Model_ (new DisplayModel (this))
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
		View_->rootContext ()->setContextProperty ("itemsModel", Model_);
		View_->setSource (QUrl ("qrc:/launchy/resources/qml/FSView.qml"));

		View_->showFullScreen ();

		View_->setFocus ();
		View_->setFocus (Qt::MouseFocusReason);

		connect (View_->rootObject (),
				SIGNAL (closeRequested ()),
				this,
				SLOT (deleteLater ()));

		handleFinderUpdated ();
	}

	FSDisplayer::~FSDisplayer ()
	{
		delete View_;
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

	void FSDisplayer::handleFinderUpdated ()
	{
		Model_->clear ();

		IconsProvider_->Clear ();

		const auto& currentLang = Util::GetLanguage ().toLower ();

		static const CategoriesInfo cInfo;

		const auto& items = Finder_->GetItems ();
		QMap<QString, QStandardItem*> visibleItems;
		QMap<QString, QMap<QString, QStandardItem*>> itemsInCats;
		for (const auto& cat : items.keys ())
		{
			if (!cInfo.Infos_.contains (cat))
			{
				qDebug () << Q_FUNC_INFO << "skipping" << cat;
				continue;
			}

			const auto& catInfo = cInfo.Infos_ [cat];
			if (!catInfo.IconName_.isEmpty ())
				IconsProvider_->AddIcon (catInfo.IconName_, Proxy_->GetIcon (catInfo.IconName_));

			const auto& visibleName = catInfo.TranslatedName_;

			QStandardItem *catItem = 0;
			if (visibleItems.contains (visibleName))
				catItem = visibleItems [visibleName];
			else
			{
				catItem = new QStandardItem ();
				catItem->setData (visibleName, DisplayModel::Roles::CategoryName);
				catItem->setData (catInfo.IconName_, DisplayModel::Roles::CategoryIcon);
			}

			auto& itemsInCat = itemsInCats [visibleName];

			for (const auto& item : items [cat])
			{
				const auto& itemName = item->GetName (currentLang);
				if (itemsInCat.contains (itemName))
					continue;

				auto appItem = new QStandardItem ();
				itemsInCat [itemName] = appItem;
				appItem->setData (itemName, DisplayModel::Roles::ItemName);

				auto comment = item->GetComment (currentLang);
				if (comment.isEmpty ())
					comment = item->GetGenericName (currentLang);
				appItem->setData (comment, DisplayModel::Roles::ItemDescription);

				const auto& iconName = item->GetIconName ();
				appItem->setData (iconName, DisplayModel::Roles::ItemIcon);

				IconsProvider_->AddIcon (iconName, item->GetIcon ());
			}

			visibleItems [visibleName] = catItem;
		}

		for (const auto& vis : visibleItems.keys ())
		{
			auto visItem = visibleItems [vis];
			visItem->appendRows (itemsInCats [vis].values ());
			Model_->appendRow (visItem);
		}
	}
}
}
