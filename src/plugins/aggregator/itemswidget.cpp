/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "itemswidget.h"
#include <memory>
#include <algorithm>
#include <limits>
#include <QFileInfo>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QUrl>
#include <QTimer>
#include <QMessageBox>
#include <QToolBar>
#include <QClipboard>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include <util/tags/categoryselector.h>
#include <util/xpc/util.h>
#include <util/models/mergemodel.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/util.h>
#include <util/sll/containerconversions.h>
#include <util/sll/curry.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "components/actions/appwideactions.h"
#include "components/actions/channelactions.h"
#include "components/gui/itemnavigator.h"
#include "xmlsettingsmanager.h"
#include "itemsfiltermodel.h"
#include "itemslistmodel.h"
#include "channelsmodel.h"
#include "uistatepersist.h"
#include "storagebackendmanager.h"
#include "itemutils.h"
#include "dbutils.h"

namespace LC
{
namespace Aggregator
{
	using LC::Util::CategorySelector;

	struct UiInit
	{
		UiInit (auto&& ui, QWidget *parent)
		{
			ui.setupUi (parent);
		}
	};

	struct ItemsWidget_Impl
	{
		ItemsWidget * const Parent_;
		Ui::ItemsWidget Ui_;

		QToolBar *ControlToolBar_;

		QAction *ActionHideReadItems_;
		QAction *ActionShowAsTape_;
		UiInit UiInit_ { Ui_, Parent_ };

		QAction *ActionMarkItemAsUnread_;
		QAction *ActionMarkItemAsRead_;
		QAction *ActionMarkItemAsImportant_;

		QAction *ActionPrevUnreadItem_;
		QAction *ActionPrevItem_;
		QAction *ActionNextItem_;
		QAction *ActionNextUnreadItem_;

		QAction *ActionDeleteItem_;
		QAction *ActionItemCommentsSubscribe_;
		QAction *ActionItemLinkOpen_;
		QAction *ActionItemLinkCopy_;

		bool TapeMode_ = false;
		bool MergeMode_ = false;

		QAbstractItemModel *ChannelsModel_ = nullptr;

		std::unique_ptr<ItemsListModel> CurrentItemsModel_ {};
		QList<std::shared_ptr<ItemsListModel>> SupplementaryModels_ {};
		std::unique_ptr<Util::MergeModel> ItemLists_ {};
		std::unique_ptr<ItemsFilterModel> ItemsFilterModel_ {};
		std::unique_ptr<CategorySelector> ItemCategorySelector_ {};

		QTimer *SelectedChecker_ = nullptr;

		// The last selected index into the ItemLists_ model.
		QModelIndex LastSelectedIndex_ {};

		QModelIndex LastSelectedChannel_ {};

		UpdatesManager *UpdatesManager_ = nullptr;

		ItemNavigator ItemNavigator_
		{
			*Ui_.Items_,
			LastSelectedIndex_,
			[this] (const QModelIndex& newChan)
			{
				emit Parent_->movedToChannel (newChan);
				Parent_->CurrentChannelChanged (newChan);
			}
		};
	};

	ItemsWidget::ItemsWidget (const Dependencies& deps, QWidget *parent)
	: QWidget (parent)
	, Impl_ (new ItemsWidget_Impl { this })
	{
		Impl_->SelectedChecker_ = new QTimer (this);
		Impl_->SelectedChecker_->setSingleShot (true);
		connect (Impl_->SelectedChecker_,
				&QTimer::timeout,
				this,
				&ItemsWidget::checkSelected);

		Impl_->UpdatesManager_ = &deps.UpdatesManager_;

		auto& cm = deps.ChannelsModel_;
		Impl_->ChannelsModel_ = &cm;
		connect (&cm,
				&QAbstractItemModel::rowsInserted,
				this,
				&ItemsWidget::invalidateMergeMode);
		connect (&cm,
				&QAbstractItemModel::rowsRemoved,
				this,
				&ItemsWidget::invalidateMergeMode);

		SetupActions ();
		auto addAct = [this, &deps] (ItemsWidget::Action actId)
		{
			auto act = GetAction (actId);
			deps.ShortcutsMgr_.RegisterAction (act->objectName (), act);
		};

		for (int i = 0; i < static_cast<int> (ItemsWidget::Action::MaxAction); ++i)
			addAct (static_cast<ItemsWidget::Action> (i));

		Impl_->TapeMode_ = XmlSettingsManager::Instance ()->
				Property ("ShowAsTape", false).toBool ();
		Impl_->MergeMode_ = false;
		Impl_->ControlToolBar_ = SetupToolBar ();

		auto first = Impl_->ControlToolBar_->actions ().first ();

		Impl_->ControlToolBar_->insertActions (first, deps.AppWideActions_.GetFastActions ());
		Impl_->ControlToolBar_->insertSeparator (first);

		Impl_->ControlToolBar_->insertActions (first, deps.ChannelActions_.GetToolbarActions ());
		Impl_->ControlToolBar_->insertSeparator (first);

		const auto& proxy = GetProxyHolder ();
		Impl_->CurrentItemsModel_ = std::make_unique<ItemsListModel> (proxy->GetIconThemeManager ());
		Impl_->ItemLists_ = std::make_unique<Util::MergeModel> (QStringList { tr ("Name"), tr ("Date") });
		Impl_->ItemLists_->AddModel (Impl_->CurrentItemsModel_.get ());

		Impl_->Ui_.setupUi (this);

		Impl_->Ui_.Items_->setAcceptDrops (false);

		Impl_->ItemsFilterModel_ = std::make_unique<ItemsFilterModel> (this);
		Impl_->ItemsFilterModel_->SetItemsWidget (this);
		Impl_->ItemsFilterModel_->setSourceModel (Impl_->ItemLists_.get ());
		Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());
		Impl_->Ui_.Items_->sortByColumn (1, Qt::DescendingOrder);
		connect (Impl_->ItemLists_.get (),
				&QAbstractItemModel::dataChanged,
				Impl_->ItemsFilterModel_.get (),
				&QSortFilterProxyModel::invalidate);

		Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsUnread_);
		Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsRead_);
		Impl_->Ui_.Items_->addAction (Util::CreateSeparator (this));
		Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsImportant_);
		Impl_->Ui_.Items_->addAction (Util::CreateSeparator (this));
		Impl_->Ui_.Items_->addAction (Impl_->ActionDeleteItem_);
		Impl_->Ui_.Items_->addAction (Util::CreateSeparator (this));
		Impl_->Ui_.Items_->addAction (Impl_->ActionItemCommentsSubscribe_);
		Impl_->Ui_.Items_->addAction (Impl_->ActionItemLinkOpen_);
		Impl_->Ui_.Items_->addAction (Impl_->ActionItemLinkCopy_);
		Impl_->Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);

		addActions ({
				Impl_->ActionPrevUnreadItem_,
				Impl_->ActionPrevItem_,
				Impl_->ActionNextItem_,
				Impl_->ActionNextUnreadItem_
			});

		connect (Impl_->Ui_.SearchLine_,
				&QLineEdit::textChanged,
				this,
				&ItemsWidget::updateItemsFilter);
		connect (Impl_->Ui_.SearchType_,
				qOverload<int> (&QComboBox::currentIndexChanged),
				this,
				&ItemsWidget::updateItemsFilter);

		new Util::ClearLineEditAddon (proxy, Impl_->Ui_.SearchLine_);

		const auto header = Impl_->Ui_.Items_->header ();
		const auto& fm = fontMetrics ();
		header->resizeSection (0,
				fm.horizontalAdvance ("Average news article size is about this width or maybe bigger, because they are bigger"));
		header->resizeSection (1,
				fm.horizontalAdvance (QLocale {}.toString (QDateTime::currentDateTime (), QLocale::ShortFormat) + "__"));
		connect (Impl_->Ui_.Items_->header (),
				&QHeaderView::sectionClicked,
				this,
				&ItemsWidget::makeCurrentItemVisible);

		Impl_->ItemCategorySelector_ = std::make_unique<CategorySelector> ();
		Impl_->ItemCategorySelector_->SetCaption (tr ("Items categories"));
		Impl_->ItemCategorySelector_->setWindowFlags (Qt::Widget);
		Impl_->Ui_.CategoriesSplitter_->addWidget (Impl_->ItemCategorySelector_.get ());
		Impl_->ItemCategorySelector_->hide ();
		Impl_->ItemCategorySelector_->setMinimumHeight (0);
		Impl_->ItemCategorySelector_->SetButtonsMode (CategorySelector::ButtonsMode::NoButtons);
		connect (Impl_->ItemCategorySelector_.get (),
				&CategorySelector::tagsSelectionChanged,
				Impl_->ItemsFilterModel_.get (),
				&ItemsFilterModel::categorySelectionChanged);

		connect (Impl_->Ui_.Items_->selectionModel (),
				&QItemSelectionModel::selectionChanged,
				this,
				&ItemsWidget::currentItemChanged);
		connect (Impl_->ItemsFilterModel_.get (),
				&ItemsFilterModel::modelReset,
				this,
				&ItemsWidget::currentItemChanged);

		currentItemChanged ();

		XmlSettingsManager::Instance ()->RegisterObject ("ShowCategorySelector",
				this, "selectorVisiblityChanged");
		XmlSettingsManager::Instance ()->RegisterObject ("ShowNavBarInItemsView",
				this, "navBarVisibilityChanged");
		selectorVisiblityChanged ();

		on_ActionHideReadItems__triggered ();
	}

	ItemsWidget::~ItemsWidget ()
	{
		on_CategoriesSplitter__splitterMoved ();

		disconnect (Impl_->ItemsFilterModel_.get (),
				0,
				this,
				0);
		disconnect (Impl_->ItemCategorySelector_.get (),
				0,
				this,
				0);
		delete Impl_;
	}

	Item ItemsWidget::GetItem (const QModelIndex& index) const
	{
		auto mapped = Impl_->ItemLists_->mapToSource (index);
		if (!mapped.isValid ())
			return {};

		auto model = static_cast<const ItemsListModel*> (mapped.model ());
		const auto& shortItem = model->GetItem (mapped);
		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		return sb->GetItem (shortItem.ItemID_).value_or (Item {});
	}

	QToolBar* ItemsWidget::GetToolBar () const
	{
		return Impl_->ControlToolBar_;
	}

	QModelIndex ItemsWidget::GetUnfilteredSelectedIndex () const
	{
		return Impl_->LastSelectedIndex_;
	}

	QAction* ItemsWidget::GetAction (Action action) const
	{
		switch (action)
		{
		case Action::MarkAsRead:
			return Impl_->ActionMarkItemAsRead_;
		case Action::MarkAsUnread:
			return Impl_->ActionMarkItemAsUnread_;
		case Action::MarkAsImportant:
			return Impl_->ActionMarkItemAsImportant_;
		case Action::PrevUnreadItem:
			return Impl_->ActionPrevUnreadItem_;
		case Action::PrevItem:
			return Impl_->ActionPrevItem_;
		case Action::NextItem:
			return Impl_->ActionNextItem_;
		case Action::NextUnreadItem:
			return Impl_->ActionNextUnreadItem_;
		case Action::Delete:
			return Impl_->ActionDeleteItem_;
		case Action::OpenLink:
			return Impl_->ActionItemLinkOpen_;
		case Action::CopyLink:
			return Impl_->ActionItemLinkCopy_;
		case Action::MaxAction:
			break;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown action"
				<< static_cast<int> (action);

		return nullptr;
	}

	void ItemsWidget::SetTapeMode (bool tape)
	{
		Impl_->TapeMode_ = tape;
		if (tape)
			disconnect (Impl_->Ui_.Items_->selectionModel (),
					&QItemSelectionModel::selectionChanged,
					this,
					&ItemsWidget::currentItemChanged);
		else
			connect (Impl_->Ui_.Items_->selectionModel (),
					&QItemSelectionModel::selectionChanged,
					this,
					&ItemsWidget::currentItemChanged);
		currentItemChanged ();

		XmlSettingsManager::Instance ()->
				setProperty ("ShowAsTape", tape);
	}

	void ItemsWidget::SetMergeMode (bool merge)
	{
		Impl_->MergeMode_ = merge;
		ClearSupplementaryModels ();

		if (Impl_->MergeMode_)
		{
			auto cm = Impl_->ChannelsModel_;
			for (int i = 0, size = cm->rowCount (); i < size; ++i)
			{
				auto index = cm->index (i, 0);
				AddSupplementaryModelFor (index.data (ChannelRoles::ChannelID).value<IDType_t> ());
			}
		}
	}

	void ItemsWidget::SetMergeModeTags (const QStringList& tags)
	{
		if (Impl_->MergeMode_)
			return;

		ClearSupplementaryModels ();

		const auto& tagsSet = Util::AsSet (tags);

		bool added = false;

		const auto cm = Impl_->ChannelsModel_;
		for (int i = 0, size = cm->rowCount (); i < size; ++i)
		{
			const auto& index = cm->index (i, 0);
			const auto& thisSet = index.data (RoleTags).toStringList ();
			if (std::none_of (thisSet.begin (), thisSet.end (),
					[&tagsSet] (const QString& tag) { return tagsSet.contains (tag); }))
				continue;

			auto cid = index.data (ChannelRoles::ChannelID).value<IDType_t> ();

			/** So that first one gets assigned to the
			 * current items model.
			 */
			if (!added)
			{
				Impl_->CurrentItemsModel_->Reset (cid);
				added = true;
			}
			else
				AddSupplementaryModelFor (cid);
		}
	}

	void ItemsWidget::SetHideRead (bool hide)
	{
		Impl_->ItemsFilterModel_->SetHideRead (hide);
	}

	void ItemsWidget::Selected (const QModelIndex& index)
	{
		Impl_->LastSelectedIndex_ = index;

		Impl_->SelectedChecker_->stop ();
		const auto timeout = XmlSettingsManager::Instance ()->
				property ("MarkAsReadTimeout").toInt () * 1000;
		Impl_->SelectedChecker_->start (timeout);
	}

	void ItemsWidget::MarkItemReadStatus (const QModelIndex& idx, bool read)
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->SetItemUnread (idx.data (ItemsListModel::ItemId).value<IDType_t> (), !read);
	}

	IDType_t ItemsWidget::GetItemIDFromRow (int index) const
	{
		ItemsListModel *model = 0;
		if (!Impl_->SupplementaryModels_.size ())
			model = Impl_->CurrentItemsModel_.get ();
		else
		{
			int starting = 0;
			const auto i = Impl_->ItemLists_->GetModelForRow (index, &starting);
			model = static_cast<ItemsListModel*> (i->data ());
			index -= starting;
		}

		return model->GetItem (model->index (index, 0)).ItemID_;
	}

	void ItemsWidget::SubscribeToComments (const QModelIndex& index) const
	{
		const auto& it = GetItem (index);
		QString commentRSS = it.CommentsLink_;
		QStringList tags = it.Categories_;

		const auto itm = GetProxyHolder ()->GetTagsManager ();
		const auto& addTags = itm->Split (XmlSettingsManager::Instance ()->property ("CommentsTags").toString ());
		AddFeed ({ .URL_ = commentRSS, .Tags_ = tags + addTags, .UpdatesManager_ = *Impl_->UpdatesManager_ });
	}

	void ItemsWidget::CurrentChannelChanged (const QModelIndex& si)
	{
		if (Impl_->MergeMode_)
			return;

		ClearSupplementaryModels ();

		Impl_->LastSelectedChannel_ = si;

		if (si.isValid ())
			Impl_->CurrentItemsModel_->Reset (si.data (ChannelRoles::ChannelID).value<IDType_t> ());
		else
			Impl_->CurrentItemsModel_->Reset (IDNotFound);

		Impl_->Ui_.Items_->scrollToTop ();
		currentItemChanged ();

		if (!isVisible ())
			return;

		const auto& items = Impl_->CurrentItemsModel_->GetAllItems ();
		const auto& allCategories = ItemUtils::GetCategories (items).values ();
		Impl_->ItemsFilterModel_->categorySelectionChanged (allCategories);

		if (!allCategories.isEmpty ())
		{
			Impl_->ItemCategorySelector_->SetPossibleSelections (allCategories);
			if (XmlSettingsManager::Instance ()->property ("ShowCategorySelector").toBool ())
				Impl_->ItemCategorySelector_->show ();
			RestoreSplitter ();
		}
		else
		{
			Impl_->ItemCategorySelector_->SetPossibleSelections ({});
			Impl_->ItemCategorySelector_->hide ();
		}
	}

	void ItemsWidget::ConstructBrowser ()
	{
		const auto browser = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<IWebBrowser*> ().value (0);
		Impl_->Ui_.ItemView_->Construct (browser);
		navBarVisibilityChanged ();
	}

	void ItemsWidget::LoadUIState ()
	{
		LoadColumnWidth (Impl_->Ui_.Items_, "items");
	}

	void ItemsWidget::SaveUIState ()
	{
		SaveColumnWidth (Impl_->Ui_.Items_, "items");
	}

	void ItemsWidget::ClearSupplementaryModels ()
	{
		while (Impl_->SupplementaryModels_.size ())
		{
			Impl_->ItemLists_->RemoveModel (Impl_->SupplementaryModels_.at (0).get ());
			Impl_->SupplementaryModels_.removeAt (0);
		}
	}

	void ItemsWidget::AddSupplementaryModelFor (IDType_t channelId)
	{
		if (channelId == Impl_->CurrentItemsModel_->GetCurrentChannel ())
			return;

		auto ilm = std::make_shared<ItemsListModel> (GetProxyHolder ()->GetIconThemeManager ());
		ilm->Reset (channelId);
		Impl_->SupplementaryModels_ << ilm;
		Impl_->ItemLists_->AddModel (ilm.get ());
	}

	void ItemsWidget::SetupActions ()
	{
		Impl_->ActionHideReadItems_ = new QAction (tr ("Hide read items"),
				this);
		Impl_->ActionHideReadItems_->setObjectName ("ActionHideReadItems_");
		Impl_->ActionHideReadItems_->setCheckable (true);
		Impl_->ActionHideReadItems_->setProperty ("ActionIcon", "mail-mark-unread");
		Impl_->ActionHideReadItems_->setChecked (XmlSettingsManager::Instance ()->
				Property ("HideReadItems", false).toBool ());

		Impl_->ActionShowAsTape_ = new QAction (tr ("Show items as tape"), this);
		Impl_->ActionShowAsTape_->setObjectName ("ActionShowAsTape_");
		Impl_->ActionShowAsTape_->setCheckable (true);
		Impl_->ActionShowAsTape_->setProperty ("ActionIcon", "format-list-unordered");
		Impl_->ActionShowAsTape_->setChecked (XmlSettingsManager::Instance ()->
				Property ("ShowAsTape", false).toBool ());

		Impl_->ActionMarkItemAsUnread_ = new QAction (tr ("Mark item as unread"), this);
		Impl_->ActionMarkItemAsUnread_->setObjectName ("ActionMarkItemAsUnread_");
		Impl_->ActionMarkItemAsUnread_->setShortcut ({ "U" });

		Impl_->ActionMarkItemAsRead_ = new QAction (tr ("Mark item as read"), this);
		Impl_->ActionMarkItemAsRead_->setObjectName ("ActionMarkItemAsRead_");
		Impl_->ActionMarkItemAsRead_->setShortcut ({ "R" });

		Impl_->ActionMarkItemAsImportant_ = new QAction (tr ("Important"), this);
		Impl_->ActionMarkItemAsImportant_->setObjectName ("ActionMarkItemAsImportant_");
		Impl_->ActionMarkItemAsImportant_->setProperty ("ActionIcon", "rating");
		Impl_->ActionMarkItemAsImportant_->setCheckable (true);
		Impl_->ActionMarkItemAsImportant_->setShortcut ({ "I" });

		Impl_->ActionPrevUnreadItem_ = new QAction (tr ("Previous unread item"), this);
		Impl_->ActionPrevUnreadItem_->setObjectName ("ActionPrevUnreadItem_");
		Impl_->ActionPrevUnreadItem_->setProperty ("ActionIcon", "go-first");
		Impl_->ActionPrevUnreadItem_->setShortcut ({ "Shift+K" });

		Impl_->ActionPrevItem_ = new QAction (tr ("Previous item"), this);
		Impl_->ActionPrevItem_->setObjectName ("ActionPrevItem_");
		Impl_->ActionPrevItem_->setProperty ("ActionIcon", "go-previous");
		Impl_->ActionPrevItem_->setShortcut ({ "K" });

		Impl_->ActionNextItem_ = new QAction (tr ("Next item"), this);
		Impl_->ActionNextItem_->setObjectName ("ActionNextItem_");
		Impl_->ActionNextItem_->setProperty ("ActionIcon", "go-next");
		Impl_->ActionNextItem_->setShortcut ({ "J" });

		Impl_->ActionNextUnreadItem_ = new QAction (tr ("Next unread item"), this);
		Impl_->ActionNextUnreadItem_->setObjectName ("ActionNextUnreadItem_");
		Impl_->ActionNextUnreadItem_->setProperty ("ActionIcon", "go-last");
		Impl_->ActionNextUnreadItem_->setShortcut ({ "Shift+J" });

		Impl_->ActionDeleteItem_ = new QAction (tr ("Delete"), this);
		Impl_->ActionDeleteItem_->setObjectName ("ActionDeleteItem_");
		Impl_->ActionDeleteItem_->setProperty ("ActionIcon", "remove");
		Impl_->ActionDeleteItem_->setShortcut ({ "Delete" });

		Impl_->ActionItemCommentsSubscribe_ = new QAction (tr ("Subscribe to comments"), this);
		Impl_->ActionItemCommentsSubscribe_->setObjectName ("ActionItemCommentsSubscribe_");

		Impl_->ActionItemLinkOpen_ = new QAction (tr ("Open in new tab"), this);
		Impl_->ActionItemLinkOpen_->setProperty ("ActionIcon", "internet-web-browser");
		Impl_->ActionItemLinkOpen_->setShortcut ({ "O" });
		Impl_->ActionItemLinkOpen_->setObjectName ("ActionItemLinkOpen_");

		Impl_->ActionItemLinkCopy_ = new QAction (tr ("Copy news item link"), this);
		Impl_->ActionItemLinkCopy_->setProperty ("ActionIcon", "edit-copy");
		Impl_->ActionItemLinkCopy_->setShortcut ({ "C" });
		Impl_->ActionItemLinkCopy_->setObjectName ("ActionItemLinkCopy_");
	}

	QToolBar* ItemsWidget::SetupToolBar ()
	{
		QToolBar *bar = new QToolBar ();
		bar->setWindowTitle ("Aggregator");
		bar->addAction (Impl_->ActionHideReadItems_);
		bar->addAction (Impl_->ActionShowAsTape_);

		return bar;
	}

	QString ItemsWidget::GetHex (QPalette::ColorRole role, QPalette::ColorGroup group)
	{
		static_assert (std::numeric_limits<int>::max () >= 0xffffffL, "int is too small :(");

		int r, g, b;
		QApplication::palette ().color (group, role).getRgb (&r, &g, &b);
		int color = b + (g << 8) + (r << 16);
		QString result ("#%1");
		// Fill spare space with zeros.
		return result.arg (color, 6, 16, QChar ('0'));
	}

	QString ItemsWidget::ToHtml (const Item& item)
	{
		QString headerBg = GetHex (QPalette::Window);
		QString borderColor = headerBg;
		QString headerText = GetHex (QPalette::WindowText);
		QString alternateBg = GetHex (QPalette::AlternateBase);

		QString firstStartBox = "<div style='background: %1; "
			"color: COLOR; "
			"padding-left: 2em; "
			"padding-right: 2em; "
			"padding-bottom: 0.5em;"
			"border: 2px none green; "
			"margin: 0px; "
			"-webkit-border-top-left-radius: 1em; "
			"-webkit-border-top-right-radius: 1em;'>";
		firstStartBox.replace ("COLOR", headerText);

		bool linw = XmlSettingsManager::Instance ()->
				property ("AlwaysUseExternalBrowser").toBool ();

		QString result = QString (
				"<style>a { color: %2; } a.visited { color: %3 }</style>"
				"<div style='background: %1; "
				"margin-top: 0em; "
				"margin-left: 0em; "
				"margin-right: 0em; "
				"margin-bottom: 0.5 em; "
				"padding: 0px; "
				"border: 2px solid %4; "
				"-webkit-border-radius: 1em;'>")
			.arg (GetHex (QPalette::Base))
			.arg (GetHex (QPalette::Link))
			.arg (GetHex (QPalette::LinkVisited))
			.arg (borderColor);

		QString inpad = QString ("<div style='background: %1; "
				"color: %2; "
				"border: 1px solid #333333; "
				"padding-top: 0.2em; "
				"padding-bottom: 0.2em; "
				"padding-left: 2em; "
				"padding-right: 2em;"
				"-webkit-border-radius: 1em;'>");

		result += firstStartBox.arg (headerBg);

		// Link
		result += ("<a href='" +
				item.Link_ +
				"'");
		if (linw)
			result += " target='_blank'";
		result += QString (">");
		result += (QString ("<strong>") +
				item.Title_ +
				"</strong>" +
				"</a><br />");

		// Publication date and author
		if (item.PubDate_.isValid () && !item.Author_.isEmpty ())
			result += tr ("Published on %1 by %2")
					.arg (item.PubDate_.toString ())
					.arg (item.Author_) +
				"<br />";
		else if (item.PubDate_.isValid ())
			result += tr ("Published on %1")
					.arg (item.PubDate_.toString ()) +
				"<br />";
		else if (!item.Author_.isEmpty ())
			result += tr ("Published by %1")
					.arg (item.Author_) +
				"<br />";

		// Categories
		if (item.Categories_.size ())
			result += item.Categories_.join ("; ") +
				"<br />";

		// Comments stuff
		if (item.NumComments_ >= 0 && !item.CommentsPageLink_.isEmpty ())
			result += tr ("%n comment(s), <a href='%1'%2>view them</a><br />",
					"", item.NumComments_)
					.arg (item.CommentsPageLink_)
					.arg (linw ? " target='_blank'" : "");
		else if (item.NumComments_ >= 0)
			result += tr ("%n comment(s)", "", item.NumComments_) +
				"<br />";
		else if (!item.CommentsPageLink_.isEmpty ())
			result += tr ("<a href='%1'%2>View comments</a><br />")
					.arg (item.CommentsPageLink_)
					.arg (linw ? " target='_blank'" : "");

		if (item.Latitude_ ||
				item.Longitude_)
		{
			QString link = QString ("http://maps.google.com/maps"
					"?f=q&source=s_q&hl=en&geocode=&q=%1+%2")
				.arg (item.Latitude_)
				.arg (item.Longitude_);
			result += tr ("Geoposition: <a href='%3'%4 title='Google Maps'>%1 %2</a><br />")
					.arg (item.Latitude_)
					.arg (item.Longitude_)
					.arg (link)
					.arg (linw ? " target='_blank'" : "");
		}

		// Description
		result += QString ("</div><div style='color: %1;"
				"padding-top: 0.5em; "
				"padding-left: 1em; "
				"padding-right: 1em;'>")
			.arg (GetHex (QPalette::Text));
		result += item.Description_;

		const auto embedImages = XmlSettingsManager::Instance ()->
				property ("EmbedMediaRSSImages").toBool ();
		for (const auto& enclosure : item.Enclosures_)
		{
			result += inpad.arg (headerBg)
				.arg (headerText);

			if (embedImages && enclosure.Type_.startsWith ("image/"))
				result += QString ("<img src='%1' /><br/>")
						.arg (enclosure.URL_);

			if (enclosure.Length_ > 0)
				result += tr ("File of type %1, size %2:<br />")
					.arg (enclosure.Type_)
					.arg (Util::MakePrettySize (enclosure.Length_));
			else
				result += tr ("File of type %1 and unknown length:<br />")
					.arg (enclosure.Type_);

			result += QString ("<a href='%1'>%2</a>")
				.arg (enclosure.URL_)
				.arg (QFileInfo (QUrl (enclosure.URL_).path ()).fileName ());
			if (!enclosure.Lang_.isEmpty ())
				result += tr ("<br />Specified language: %1")
					.arg (enclosure.Lang_);
			result += "</div>";
		}

		for (QList<MRSSEntry>::const_iterator entry = item.MRSSEntries_.begin (),
				endEntry = item.MRSSEntries_.end (); entry != endEntry; ++entry)
		{
			result += inpad.arg (headerBg)
				.arg (headerText);

			QString url = entry->URL_;

			if (entry->Medium_ == "image")
				result += tr ("Image") + ' ';
			else if (entry->Medium_ == "audio")
				result += tr ("Audio") + ' ';
			else if (entry->Medium_ == "video")
				result += tr ("Video") + ' ';
			else if (entry->Medium_ == "document")
				result += tr ("Document") + ' ';
			else if (entry->Medium_ == "executable")
				result += tr ("Executable") + ' ';

			if (entry->Title_.isEmpty ())
				result += QString ("<a href='%1' target='_blank'>%1</a><hr />")
					.arg (url);
			else
				result += QString ("<a href='%1' target='_blank'>%2</a><hr />")
					.arg (url)
					.arg (entry->Title_);

			if (entry->Size_ > 0)
			{
				result += Util::MakePrettySize (entry->Size_);
				result += "<br />";
			}

			QString peers;
			for (const auto& pl : entry->PeerLinks_)
				peers += QString ("<li>Also available in <a href='%1'>P2P (%2)</a></li>")
					.arg (pl.Link_)
					.arg (pl.Type_);
			if (peers.size ())
			{
				result += inpad.arg (alternateBg)
					.arg (headerText);
				result += QString ("<ul>%1</ul>")
					.arg (peers);
				result += "</div>";
			}

			if (!entry->Description_.isEmpty ())
				result += QString ("%1<br />")
					.arg (entry->Description_);

			QList<int> sizes;
			int num = 0;
			for (int i = 0; i < entry->Thumbnails_.size (); ++i)
			{
				int width = entry->Thumbnails_.at (i).Width_;
				if (!width)
					break;

				if (!sizes.contains (width))
					sizes << width;
				else
				{
					bool broke = false;;
					for (int j = i + 1; j < entry->Thumbnails_.size (); ++j)
						if (entry->Thumbnails_.at (j).Width_ == sizes.at (j % sizes.size ()))
						{
							broke = true;
							break;
						}

					if (broke)
						continue;
					num = sizes.size ();
					break;
				}
			}

			if (!num || num == entry->Thumbnails_.size ())
				num = 3;

			int cur = 1;
			for (const auto& thumb : entry->Thumbnails_)
			{
				if (!thumb.Time_.isEmpty ())
					result += tr ("<hr />Thumbnail at %1:<br />")
						.arg (thumb.Time_);
				result += QString ("<img src='%1' ")
					.arg (thumb.URL_);
				if (thumb.Width_)
					result += QString ("width='%1' ")
						.arg (thumb.Width_);
				if (thumb.Height_)
					result += QString ("height='%1' ")
						.arg (thumb.Height_);
				result += "/>";

				if (num && cur < num)
					++cur;
				else
				{
					result += "<br />";
					cur = 1;
				}
			}

			result += "<hr />";

			if (!entry->Keywords_.isEmpty ())
				result += tr ("<strong>Keywords:</strong> <em>%1</em><br />")
					.arg (entry->Keywords_);

			if (!entry->Lang_.isEmpty ())
				result += tr ("<strong>Language:</strong> %1<br />")
					.arg (entry->Lang_);

			if (entry->Expression_ == "sample")
				result += tr ("Sample");
			else if (entry->Expression_ == "nonstop")
				result += tr ("Continuous stream");
			else
				result += tr ("Full version");
			result += "<br />";

			QString scenes;
			for (const auto& sc : entry->Scenes_)
			{
				QString current;
				if (!sc.Title_.isEmpty ())
					current += tr ("Title: %1<br />")
						.arg (sc.Title_);
				if (!sc.StartTime_.isEmpty ())
					current += tr ("Start time: %1<br />")
						.arg (sc.StartTime_);
				if (!sc.EndTime_.isEmpty ())
					current += tr ("End time: %1<br />")
						.arg (sc.EndTime_);
				if (!sc.Description_.isEmpty ())
					current += QString ("%1<br />")
						.arg (sc.Description_);

				if (!current.isEmpty ())
					scenes += QString ("<li>%1</li>")
						.arg (current);
			}

			if (scenes.size ())
			{
				result += tr ("<strong>Scenes:</strong>");
				result += inpad.arg (alternateBg)
					.arg (headerText);
				result += QString ("<ul>%1</ul>")
					.arg (scenes);
				result += "</div>";
			}

			if (entry->Views_)
				result += tr ("<strong>Views:</strong> %1")
					.arg (entry->Views_);
			if (entry->Favs_)
				result += tr ("<strong>Added to favorites:</strong> %n time(s)",
						"", entry->Favs_);
			if (entry->RatingAverage_)
				result += tr ("<strong>Average rating:</strong> %1")
					.arg (entry->RatingAverage_);
			if (entry->RatingCount_)
				result += tr ("<strong>Number of marks:</strong> %1")
					.arg (entry->RatingCount_);
			if (entry->RatingMin_)
				result += tr ("<strong>Minimal rating:</strong> %1")
					.arg (entry->RatingMin_);
			if (entry->RatingMax_)
				result += tr ("<strong>Maximal rating:</strong> %1")
					.arg (entry->RatingMax_);

			if (!entry->Tags_.isEmpty ())
				result += tr ("<strong>User tags:</strong> %1")
					.arg (entry->Tags_);

			QString tech;
			if (entry->Duration_)
				tech += tr ("<li><strong>Duration:</strong> %1</li>")
					.arg (entry->Channels_);
			if (entry->Channels_)
				tech += tr ("<li><strong>Channels:</strong> %1</li>")
					.arg (entry->Channels_);
			if (entry->Width_ &&
					entry->Height_)
				tech += tr ("<li><strong>Size:</strong> %1x%2</li>")
					.arg (entry->Width_)
					.arg (entry->Height_);
			if (entry->Bitrate_)
				tech += tr ("<li><strong>Bitrate:</strong> %1 kbps</li>")
					.arg (entry->Bitrate_);
			if (entry->Framerate_)
				tech += tr ("<li><strong>Framerate:</strong> %1</li>")
					.arg (entry->Framerate_);
			if (entry->SamplingRate_)
				tech += tr ("<li><strong>Sampling rate:</strong> %1</li>")
					.arg (entry->SamplingRate_);
			if (!entry->Type_.isEmpty ())
				tech += tr ("<li><strong>MIME type:</strong> %1</li>")
					.arg (entry->Type_);

			if (!tech.isEmpty ())
			{
				result += tr ("<strong>Technical information:</strong>");
				result += inpad.arg (alternateBg)
					.arg (headerText);
				result += QString ("<ul>%1</ul>")
					.arg (tech);
				result += "</div>";
			}

			if (!entry->Rating_.isEmpty () &&
					!entry->RatingScheme_.isEmpty ())
				result += tr ("<strong>Rating:</strong> %1 (according to %2 scheme)<br />")
					.arg (entry->Rating_)
					.arg (entry->RatingScheme_.mid (4));

			QMap<QString, QString> comments;
			for (const auto& cm : entry->Comments_)
				comments [cm.Type_] += QString ("<li>%1</li>")
					.arg (cm.Comment_);

			QStringList cmTypes = comments.keys ();
			for (const auto& type : cmTypes)
			{
				result += QString ("<strong>%1:</strong>")
					.arg (type);
				result += inpad.arg (alternateBg)
					.arg (headerText);
				result += QString ("<ul>%1</ul>")
					.arg (comments [type]);
				result += "</div>";
			}

			if (!entry->CopyrightURL_.isEmpty ())
			{
				if (!entry->CopyrightText_.isEmpty ())
					result += tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%2</a><br />")
						.arg (entry->CopyrightURL_)
						.arg (entry->CopyrightText_);
				else
					result += tr ("<strong>Copyright:</strong> <a href='%1' target='_blank'>%1</a><br />")
						.arg (entry->CopyrightURL_);
			}
			else if (!entry->CopyrightText_.isEmpty ())
				result += tr ("<strong>Copyright:</strong> %1<br />")
					.arg (entry->CopyrightText_);

			QString credits;
			for (const auto& cr : entry->Credits_)
				if (!cr.Role_.isEmpty ())
					credits += QString ("<li>%1: %2</li>")
						.arg (cr.Role_)
						.arg (cr.Who_);

			if (!credits.isEmpty ())
			{
				result += tr ("<strong>Credits:</strong>");
				result += inpad.arg (alternateBg)
					.arg (headerText);
				result += QString ("<ul>%1</ul>")
					.arg (credits);
				result += "</div>";
			}

			result += "</div>";
		}

		result += "</div>";
		result += "</div>";

		return result;
	}

	void ItemsWidget::RestoreSplitter ()
	{
		QList<int> sizes;
		sizes << XmlSettingsManager::Instance ()->
			Property ("CategoriesSplitter1", 0).toInt ();
		sizes << XmlSettingsManager::Instance ()->
			Property ("CategoriesSplitter2", 0).toInt ();
		if (!sizes.at (0) &&
				!sizes.at (1))
		{
			Impl_->Ui_.CategoriesSplitter_->setStretchFactor (0, 8);
			Impl_->Ui_.CategoriesSplitter_->setStretchFactor (1, 1);
		}
		else
			Impl_->Ui_.CategoriesSplitter_->setSizes (sizes);
	}

	QList<QPersistentModelIndex> ItemsWidget::GetSelected () const
	{
		QList<QPersistentModelIndex> result;
		for (const auto& idx : Impl_->Ui_.Items_->selectionModel ()->selectedRows ())
		{
			const auto& mapped = Impl_->ItemsFilterModel_->mapToSource (idx);
			if (!mapped.isValid ())
				continue;

			result << mapped;
		}
		return result;
	}

	void ItemsWidget::invalidateMergeMode ()
	{
		if (Impl_->MergeMode_)
		{
			SetMergeMode (false);
			SetMergeMode (true);
		}
	}

	void ItemsWidget::on_ActionHideReadItems__triggered ()
	{
		bool hide = Impl_->ActionHideReadItems_->isChecked ();
		XmlSettingsManager::Instance ()->setProperty ("HideReadItems", hide);
		SetHideRead (hide);
	}

	void ItemsWidget::on_ActionShowAsTape__triggered ()
	{
		SetTapeMode (!Impl_->TapeMode_);
	}

	void ItemsWidget::on_ActionMarkItemAsUnread__triggered ()
	{
		for (const auto& idx : GetSelected ())
			MarkItemReadStatus (idx, false);
	}

	void ItemsWidget::on_ActionMarkItemAsRead__triggered ()
	{
		for (const auto& idx : GetSelected ())
			MarkItemReadStatus (idx, true);
	}

	void ItemsWidget::on_ActionMarkItemAsImportant__triggered ()
	{
		const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();

		const bool mark = Impl_->ActionMarkItemAsImportant_->isChecked ();

		const ITagsManager::tag_id impId = "_important";

		for (const auto& idx : GetSelected ())
		{
			const auto& mapped = Impl_->ItemLists_->mapToSource (idx);
			const auto model =
					static_cast<ItemsListModel*> (Impl_->ItemLists_->
							GetModelForRow (idx.row ())->data ());

			const auto item = model->GetItem (mapped).ItemID_;

			auto tags = sb->GetItemTags (item);
			if (mark && !tags.contains (impId))
				sb->SetItemTags (item, tags + QStringList (impId));
			else if (!mark && tags.removeAll (impId))
				sb->SetItemTags (item, tags);
		}
	}

	void ItemsWidget::on_ActionDeleteItem__triggered ()
	{
		QSet<IDType_t> ids;
		for (const auto& idx : GetSelected ())
		{
			const QModelIndex& mapped = Impl_->ItemLists_->mapToSource (idx);
			const ItemsListModel *model =
					static_cast<ItemsListModel*> (Impl_->ItemLists_->
							GetModelForRow (idx.row ())->data ());
			ids << model->GetItem (mapped).ItemID_;
		}

		if (ids.isEmpty ())
			return;

		if (QMessageBox::warning (this,
					"LeechCraft",
					tr ("Are you sure you want to remove %n items?", 0, ids.size ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		Impl_->Ui_.Items_->clearSelection ();
		StorageBackendManager::Instance ().MakeStorageBackendForThread ()->RemoveItems (ids);
	}

	void ItemsWidget::on_ActionPrevUnreadItem__triggered ()
	{
		Impl_->ItemNavigator_.MoveToPrevUnread ();
	}

	void ItemsWidget::on_ActionPrevItem__triggered ()
	{
		Impl_->ItemNavigator_.MoveToPrev ();
	}

	void ItemsWidget::on_ActionNextItem__triggered ()
	{
		Impl_->ItemNavigator_.MoveToNext ();
	}

	void ItemsWidget::on_ActionNextUnreadItem__triggered ()
	{
		Impl_->ItemNavigator_.MoveToNextUnread ();
	}

	void ItemsWidget::on_CaseSensitiveSearch__stateChanged (int state)
	{
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
				Qt::CaseSensitive : Qt::CaseInsensitive);
	}

	void ItemsWidget::on_ActionItemCommentsSubscribe__triggered ()
	{
		for (const auto& idx : GetSelected ())
			SubscribeToComments (idx);
	}

	void ItemsWidget::on_ActionItemLinkOpen__triggered ()
	{
		const auto iem = GetProxyHolder ()->GetEntityManager ();
		for (const auto& idx : GetSelected ())
			iem->HandleEntity (Util::MakeEntity (QUrl { GetItem (idx).Link_ },
						{},
						FromUserInitiated | OnlyHandle));
	}

	void ItemsWidget::on_ActionItemLinkCopy__triggered ()
	{
		const auto& idx = GetSelected ().value (0);
		const auto& item = GetItem (idx);
		if (item.ItemID_ == IDNotFound)
			return;

		QApplication::clipboard ()->setText (item.Link_);
	}

	void ItemsWidget::on_CategoriesSplitter__splitterMoved ()
	{
		QList<int> sizes = Impl_->Ui_.CategoriesSplitter_->sizes ();
		XmlSettingsManager::Instance ()->
			setProperty ("CategoriesSplitter1", sizes.at (0));
		XmlSettingsManager::Instance ()->
			setProperty ("CategoriesSplitter2", sizes.at (1));
	}

	void ItemsWidget::currentItemChanged ()
	{
		const QModelIndex& current = Impl_->ItemsFilterModel_->
				mapToSource (Impl_->Ui_.Items_->selectionModel ()->currentIndex ());
		if (current.isValid ())
		{
			const int idx = GetItem (current).ItemID_;
			const auto& tags = StorageBackendManager::Instance ().MakeStorageBackendForThread ()->GetItemTags (idx);
			Impl_->ActionMarkItemAsImportant_->setChecked (tags.contains ("_important"));
		}

		QString preHtml = R"(<html><head><meta charset="UTF-8" /><title>News</title></head><body bgcolor=")";
		preHtml += palette ().color (QPalette::Base).name ();
		preHtml += "\">";
		if (Impl_->TapeMode_)
		{
			QString html;
			QUrl base;
			for (int i = 0, size = Impl_->ItemsFilterModel_->rowCount ();
					i < size; ++i)
			{
				QModelIndex index = Impl_->ItemsFilterModel_->index (i, 0);
				QModelIndex mapped = Impl_->ItemsFilterModel_->mapToSource (index);
				const auto& item = GetItem (mapped);
				if (!i)
					base = item.Link_;

				html += ToHtml (item);
			}

			Impl_->Ui_.ItemView_->SetHtml (preHtml + html + "</body></html>", base);
		}
		else
		{
			QString html;
			QUrl link;

			const auto& rows = Impl_->Ui_.Items_->selectionModel ()->selectedRows ();
			for (const auto& selIndex : rows)
			{
				const QModelIndex& sindex = Impl_->
						ItemsFilterModel_->mapToSource (selIndex);
				if (!sindex.isValid ())
					continue;

				const auto& item = GetItem (sindex);
				html += ToHtml (item);
				if (!link.isValid ())
					link = item.Link_;
			}

			Impl_->Ui_.ItemView_->SetHtml (QString (), QUrl ());
			Impl_->Ui_.ItemView_->SetHtml (preHtml + html + "</body></html>", link);

			if (html.isEmpty ())
			{
				Impl_->ActionItemCommentsSubscribe_->setEnabled (false);
				Impl_->ActionMarkItemAsUnread_->setEnabled (false);
				Impl_->ActionMarkItemAsRead_->setEnabled (false);
				Impl_->ActionItemLinkOpen_->setEnabled (false);
				Impl_->ActionItemLinkCopy_->setEnabled (false);
			}
			else
			{
				auto sourceIndex = Impl_->Ui_.Items_->currentIndex ();
				if (!sourceIndex.isValid ())
					sourceIndex = rows.value (0);

				const auto& cIndex = Impl_->ItemsFilterModel_->mapToSource (sourceIndex);

				Selected (cIndex);

				QString commentsRSS = GetItem (cIndex).CommentsLink_;
				Impl_->ActionItemCommentsSubscribe_->setEnabled (!commentsRSS.isEmpty ());

				Impl_->ActionMarkItemAsUnread_->setEnabled (true);
				Impl_->ActionMarkItemAsRead_->setEnabled (true);
				Impl_->ActionItemLinkOpen_->setEnabled (true);
				Impl_->ActionItemLinkCopy_->setEnabled (true);
			}
		}
	}

	void ItemsWidget::checkSelected ()
	{
		const auto& sourceIndex = Impl_->Ui_.Items_->currentIndex ();
		const auto& cIndex = Impl_->ItemsFilterModel_->mapToSource (sourceIndex);
		if (cIndex != Impl_->LastSelectedIndex_ || cIndex.data (IItemsModel::ItemRole::IsRead).toBool ())
			return;

		const auto sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
		sb->SetItemUnread (cIndex.data (IItemsModel::ItemRole::ItemId).value<IDType_t> (), false);
	}

	void ItemsWidget::makeCurrentItemVisible ()
	{
		const auto& item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
		if (item.isValid ())
			Impl_->Ui_.Items_->scrollTo (item);
	}

	void ItemsWidget::updateItemsFilter ()
	{
		const int section = Impl_->Ui_.SearchType_->currentIndex ();
		if (section == 4)
		{
			const auto& sb = StorageBackendManager::Instance ().MakeStorageBackendForThread ();
			Impl_->CurrentItemsModel_->Reset (sb->GetItemsForTag ("_important"));
		}
		else
			CurrentChannelChanged (Impl_->LastSelectedChannel_);

		const QString& text = Impl_->Ui_.SearchLine_->text ();
		switch (section)
		{
		case 1:
			Impl_->ItemsFilterModel_->setFilterWildcard (text);
			break;
		case 2:
			Impl_->ItemsFilterModel_->setFilterRegExp (text);
			break;
		default:
			Impl_->ItemsFilterModel_->setFilterFixedString (text);
			break;
		}

		QList<ITagsManager::tag_id> tags;
		if (section == 3)
			tags << "_important";
		Impl_->ItemsFilterModel_->SetItemTags (tags);
	}

	void ItemsWidget::selectorVisiblityChanged ()
	{
		if (!XmlSettingsManager::Instance ()->
				property ("ShowCategorySelector").toBool ())
		{
			Impl_->ItemCategorySelector_->SelectAll ();
			Impl_->ItemCategorySelector_->hide ();
		}
		else if (Impl_->ItemCategorySelector_->GetSelections ().size ())
			Impl_->ItemCategorySelector_->show ();
	}

	void ItemsWidget::navBarVisibilityChanged ()
	{
		Impl_->Ui_.ItemView_->
			SetNavBarVisible (XmlSettingsManager::Instance ()->
					property ("ShowNavBarInItemsView").toBool ());
	}
}
}
