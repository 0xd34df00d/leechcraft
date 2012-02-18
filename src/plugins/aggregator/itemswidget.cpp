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

#include "itemswidget.h"
#include <memory>
#include <algorithm>
#include <QFileInfo>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QUrl>
#include <QTimer>
#include <QtDebug>
#include <interfaces/iwebbrowser.h>
#include <util/categoryselector.h>
#include <util/util.h>
#include <util/mergemodel.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "itemsfiltermodel.h"
#include "itemslistmodel.h"
#include "channelsmodel.h"
#include "uistatepersist.h"
#include <qmessagebox.h>

namespace LeechCraft
{
namespace Aggregator
{
	using LeechCraft::Util::CategorySelector;

	struct ItemsWidget_Impl
	{
		Ui::ItemsWidget Ui_;

		QToolBar *ControlToolBar_;
		QAction *ActionHideReadItems_;
		QAction *ActionShowAsTape_;

		QAction *ActionMarkItemAsUnread_;
		QAction *ActionMarkItemAsRead_;
		QAction *ActionMarkItemAsImportant_;
		QAction *ActionDeleteItem_;
		QAction *ActionItemCommentsSubscribe_;
		QAction *ActionItemLinkOpen_;

		bool TapeMode_;
		bool MergeMode_;

		QSortFilterProxyModel *ChannelsFilter_;

		std::auto_ptr<ItemsListModel> CurrentItemsModel_;
		QList<std::shared_ptr<ItemsListModel> > SupplementaryModels_;
		std::auto_ptr<Util::MergeModel> ItemLists_;
		std::auto_ptr<ItemsFilterModel> ItemsFilterModel_;
		std::auto_ptr<CategorySelector> ItemCategorySelector_;

		QTimer *SelectedChecker_;
		QModelIndex LastSelectedIndex_;
		QModelIndex LastSelectedChannel_;
	};

	ItemsWidget::ItemsWidget (QWidget *parent)
	: QWidget (parent)
	, Impl_ (new ItemsWidget_Impl)
	{
		Impl_->SelectedChecker_ = new QTimer (this);
		Impl_->SelectedChecker_->setSingleShot (true);
		connect (Impl_->SelectedChecker_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkSelected ()));

		SetupActions ();

		Impl_->ChannelsFilter_ = 0;
		Impl_->TapeMode_ = XmlSettingsManager::Instance ()->
				Property ("ShowAsTape", false).toBool ();
		Impl_->MergeMode_ = false;
		Impl_->ControlToolBar_ = SetupToolBar ();

		Impl_->CurrentItemsModel_.reset (new ItemsListModel);
		QStringList headers;
		headers << tr ("Name")
			<< tr ("Date");
		Impl_->ItemLists_.reset (new Util::MergeModel (headers));
		Impl_->ItemLists_->AddModel (Impl_->CurrentItemsModel_.get ());

		Impl_->Ui_.setupUi (this);

		Impl_->Ui_.Items_->setAcceptDrops (false);

		Impl_->ItemsFilterModel_.reset (new ItemsFilterModel (this));
		Impl_->ItemsFilterModel_->SetItemsWidget (this);
		Impl_->ItemsFilterModel_->setSourceModel (Impl_->ItemLists_.get ());
		Impl_->ItemsFilterModel_->setFilterKeyColumn (0);
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		Impl_->Ui_.Items_->setModel (Impl_->ItemsFilterModel_.get ());
		Impl_->Ui_.Items_->sortByColumn (1, Qt::DescendingOrder);
		connect (Impl_->ItemLists_.get (),
				SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
				Impl_->ItemsFilterModel_.get (),
				SLOT (invalidate ()));

		Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsUnread_);
		Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsRead_);
		Impl_->Ui_.Items_->addAction (Util::CreateSeparator (this));
		Impl_->Ui_.Items_->addAction (Impl_->ActionMarkItemAsImportant_);
		Impl_->Ui_.Items_->addAction (Util::CreateSeparator (this));
		Impl_->Ui_.Items_->addAction (Impl_->ActionDeleteItem_);
		Impl_->Ui_.Items_->addAction (Util::CreateSeparator (this));
		Impl_->Ui_.Items_->addAction (Impl_->ActionItemCommentsSubscribe_);
		Impl_->Ui_.Items_->addAction (Impl_->ActionItemLinkOpen_);
		Impl_->Ui_.Items_->setContextMenuPolicy (Qt::ActionsContextMenu);

		connect (Impl_->Ui_.SearchLine_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (updateItemsFilter ()));
		connect (Impl_->Ui_.SearchType_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (updateItemsFilter ()));

		connect (this,
				SIGNAL (currentChannelChanged (const QModelIndex&)),
				this,
				SLOT (channelChanged (const QModelIndex&)));

		QHeaderView *itemsHeader = Impl_->Ui_.Items_->header ();
		QFontMetrics fm = fontMetrics ();
		int dateTimeSize = fm.width (QDateTime::currentDateTime ()
				.toString (Qt::SystemLocaleShortDate) + "__");
		itemsHeader->resizeSection (0,
				fm.width ("Average news article size is about this width or "
					"maybe bigger, because they are bigger"));
		itemsHeader->resizeSection (1,
				dateTimeSize);
		connect (Impl_->Ui_.Items_->header (),
				SIGNAL (sectionClicked (int)),
				this,
				SLOT (makeCurrentItemVisible ()));

		Impl_->ItemCategorySelector_.reset (new CategorySelector ());
		Impl_->ItemCategorySelector_->SetCaption (tr ("Items categories"));
		Impl_->ItemCategorySelector_->setWindowFlags (Qt::Widget);
		Impl_->Ui_.CategoriesSplitter_->addWidget (Impl_->ItemCategorySelector_.get ());
		Impl_->ItemCategorySelector_->hide ();
		Impl_->ItemCategorySelector_->setMinimumHeight (0);
		connect (Impl_->ItemCategorySelector_.get (),
				SIGNAL (selectionChanged (const QStringList&)),
				Impl_->ItemsFilterModel_.get (),
				SLOT (categorySelectionChanged (const QStringList&)));

		connect (Impl_->Ui_.Items_->selectionModel (),
				SIGNAL (selectionChanged (const QItemSelection&,
						const QItemSelection&)),
				this,
				SLOT (currentItemChanged ()));
		connect (Impl_->ItemsFilterModel_.get (),
				SIGNAL (modelReset ()),
				this,
				SLOT (currentItemChanged ()));

		currentItemChanged ();

		connect (&Core::Instance (),
				SIGNAL (itemDataUpdated (Item_ptr, Channel_ptr)),
				this,
				SLOT (handleItemDataUpdated (Item_ptr, Channel_ptr)),
				Qt::QueuedConnection);

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

	void ItemsWidget::SetAppWideActions (const AppWideActions& awa)
	{
		QAction *first = Impl_->ControlToolBar_->actions ().first ();
		Impl_->ControlToolBar_->insertAction (first,
				awa.ActionUpdateFeeds_);
		Impl_->ControlToolBar_->insertSeparator (first);
	}

	void ItemsWidget::SetChannelActions (const ChannelActions& ca)
	{
		QAction *first = Impl_->ControlToolBar_->actions ().first ();
		Impl_->ControlToolBar_->insertAction (first,
				ca.ActionRemoveFeed_);
		Impl_->ControlToolBar_->insertAction (first,
				ca.ActionUpdateSelectedFeed_);
		Impl_->ControlToolBar_->insertSeparator (first);
	}

	void ItemsWidget::SetChannelsFilter (QSortFilterProxyModel *m)
	{
		Impl_->ChannelsFilter_ = m;

		connect (m,
				SIGNAL (rowsInserted (const QModelIndex&,
						int, int)),
				this,
				SLOT (invalidateMergeMode ()));
		connect (m,
				SIGNAL (rowsRemoved (const QModelIndex&,
						int, int)),
				this,
				SLOT (invalidateMergeMode ()));
	}

	Item_ptr ItemsWidget::GetItem (const QModelIndex& index) const
	{
		QModelIndex mapped = Impl_->ItemLists_->mapToSource (index);
		const ItemsListModel *model = static_cast<const ItemsListModel*> (mapped.model ());
		ItemShort item = model->GetItem (mapped);
		return Core::Instance ().GetStorageBackend ()->
			GetItem (item.ItemID_);
	}

	QToolBar* ItemsWidget::GetToolBar () const
	{
		return Impl_->ControlToolBar_;
	}

	void ItemsWidget::SetTapeMode (bool tape)
	{
		Impl_->TapeMode_ = tape;
		if (tape)
			disconnect (Impl_->Ui_.Items_->selectionModel (),
					SIGNAL (selectionChanged (const QItemSelection&,
							const QItemSelection&)),
					this,
					SLOT (currentItemChanged ()));
		else
			connect (Impl_->Ui_.Items_->selectionModel (),
					SIGNAL (selectionChanged (const QItemSelection&,
							const QItemSelection&)),
					this,
					SLOT (currentItemChanged ()));
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
			QSortFilterProxyModel *f = Impl_->ChannelsFilter_;
			ChannelsModel *cm = Core::Instance ().GetRawChannelsModel ();
			for (int i = 0, size = f ?
					f->rowCount () :
					cm->rowCount ();
					i < size; ++i)
			{
				QModelIndex index = f ?
					f->index (i, 0) :
					cm->index (i, 0);
				ChannelShort cs;
				try
				{
					cs = cm->
						GetChannelForIndex (f ? f->mapToSource (index) : index);
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
						<< e.what ();
					continue;
				}
				AddSupplementaryModelFor (cs);
			}
		}
	}

	void ItemsWidget::SetMergeModeTags (const QStringList& tags)
	{
		if (Impl_->MergeMode_)
			return;

		ClearSupplementaryModels ();

		QSet<QString> tagsSet = QSet<QString>::fromList (tags);

		ChannelsModel *cm = Core::Instance ().GetRawChannelsModel ();
		bool added = false;

		for (int i = 0, size = cm->rowCount (); i < size; ++i)
		{
			QModelIndex index = cm->index (i, 0);
			QSet<QString> thisSet = QSet<QString>::fromList (index
					.data (RoleTags).toStringList ());
			if (!thisSet.intersect (tagsSet).size ())
				continue;

			ChannelShort cs;
			try
			{
				cs = cm-> GetChannelForIndex (index);
			}
			catch (const std::exception& e)
			{
				qWarning () << Q_FUNC_INFO << e.what ();
				continue;
			}

			/** So that first one gets assigned to the
				* current items model.
				*/
			if (!added)
			{
				Impl_->CurrentItemsModel_->Reset (cs.ChannelID_);
				added = true;
			}
			else
				AddSupplementaryModelFor (cs);
		}
	}

	void ItemsWidget::SetHideRead (bool hide)
	{
		Impl_->ItemsFilterModel_->SetHideRead (hide);
	}

	bool ItemsWidget::IsItemCurrent (int item) const
	{
		int starting = 0;
		Util::MergeModel::const_iterator i = Impl_->ItemLists_->
			GetModelForRow (item, &starting);
		return static_cast<ItemsListModel*> (i->data ())->
			GetSelectedRow () == item - starting;
	}

	void ItemsWidget::Selected (const QModelIndex& index)
	{
		Impl_->LastSelectedIndex_ = index;
		Impl_->SelectedChecker_->stop ();
		int timeout = XmlSettingsManager::Instance ()->
				property ("MarkAsReadTimeout").toInt () * 1000;
		if (timeout)
			Impl_->SelectedChecker_->start (timeout);
		else
			checkSelected ();
	}

	void ItemsWidget::MarkItemReadStatus (const QModelIndex& i, bool read)
	{
		QModelIndex mapped = Impl_->ItemLists_->mapToSource (i);
		static_cast<ItemsListModel*> (Impl_->ItemLists_->
				GetModelForRow (i.row ())->data ())->
						MarkItemReadStatus (mapped, read);
	}

	bool ItemsWidget::IsItemRead (int item) const
	{
		int starting = 0;
		Util::MergeModel::const_iterator i = Impl_->ItemLists_->
			GetModelForRow (item, &starting);
		return static_cast<ItemsListModel*> (i->data ())->IsItemRead (item - starting);
	}

	bool ItemsWidget::IsItemReadNotCurrent (int item) const
	{
		int starting = 0;
		Util::MergeModel::const_iterator i = Impl_->ItemLists_->
			GetModelForRow (item, &starting);
		ItemsListModel *m = static_cast<ItemsListModel*> (i->data ());
		return m->IsItemRead (item - starting) &&
				m->GetSelectedRow () != item - starting;
	}

	QStringList ItemsWidget::GetItemCategories (int index) const
	{
		if (!Impl_->SupplementaryModels_.size ())
			return Impl_->CurrentItemsModel_->GetCategories (index);
		else
		{
			int starting = 0;
			LeechCraft::Util::MergeModel::const_iterator i = Impl_->ItemLists_->
				GetModelForRow (index, &starting);
			return static_cast<ItemsListModel*> (i->data ())->GetCategories (index - starting);
		}
	}

	IDType_t ItemsWidget::GetItemIDFromRow (int index) const
	{
		ItemsListModel *model = 0;
		if (!Impl_->SupplementaryModels_.size ())
			model = Impl_->CurrentItemsModel_.get ();
		else
		{
			int starting = 0;
			LeechCraft::Util::MergeModel::const_iterator i = Impl_->ItemLists_->
				GetModelForRow (index, &starting);
			model = static_cast<ItemsListModel*> (i->data ());
			index -= starting;
		}

		return model->GetItem (model->index (index, 0)).ItemID_;
	}

	void ItemsWidget::SubscribeToComments (const QModelIndex& index) const
	{
		Item_ptr it = GetItem (index);
		QString commentRSS = it->CommentsLink_;
		QStringList tags = it->Categories_;

		QStringList addTags = Core::Instance ().GetProxy ()->
			GetTagsManager ()->Split (XmlSettingsManager::Instance ()->
					property ("CommentsTags").toString ());
		Core::Instance ().AddFeed (commentRSS, tags + addTags);
	}

	void ItemsWidget::CurrentChannelChanged (const QModelIndex& si)
	{
		if (Impl_->MergeMode_)
			return;

		ClearSupplementaryModels ();

		Impl_->LastSelectedChannel_ = si;

		QModelIndex index = si;
		QSortFilterProxyModel *f = Impl_->ChannelsFilter_;
		if (f)
			index = f->mapToSource (index);

		try
		{
			ChannelShort ch = Core::Instance ()
				.GetRawChannelsModel ()->GetChannelForIndex (index);
			Impl_->CurrentItemsModel_->Reset (ch.ChannelID_);
		}
		catch (const std::exception&)
		{
			Impl_->LastSelectedChannel_ = QModelIndex ();
			Impl_->CurrentItemsModel_->Reset (-1);
		}
		emit currentChannelChanged (index);
	}

	void ItemsWidget::ConstructBrowser ()
	{
		Impl_->Ui_.ItemView_->Construct (Core::Instance ().GetWebBrowser ());
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
			Impl_->ItemLists_->
					RemoveModel (Impl_->SupplementaryModels_.at (0).get ());
			Impl_->SupplementaryModels_.removeAt (0);
		}
	}

	void ItemsWidget::AddSupplementaryModelFor (const ChannelShort& cs)
	{
		if (cs.ChannelID_ == Impl_->CurrentItemsModel_->GetCurrentChannel ())
			return;

		std::shared_ptr<ItemsListModel> ilm (new ItemsListModel);
		ilm->Reset (cs.ChannelID_);
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

		Impl_->ActionShowAsTape_ = new QAction (tr ("Show items as tape"),
				this);
		Impl_->ActionShowAsTape_->setObjectName ("ActionShowAsTape_");
		Impl_->ActionShowAsTape_->setCheckable (true);
		Impl_->ActionShowAsTape_->setProperty ("ActionIcon", "format-list-unordered");
		Impl_->ActionShowAsTape_->setChecked (XmlSettingsManager::Instance ()->
				Property ("ShowAsTape", false).toBool ());

		Impl_->ActionMarkItemAsUnread_ = new QAction (tr ("Mark item as unread"),
				this);
		Impl_->ActionMarkItemAsUnread_->setObjectName ("ActionMarkItemAsUnread_");

		Impl_->ActionMarkItemAsRead_ = new QAction (tr ("Mark item as read"),
				this);
		Impl_->ActionMarkItemAsRead_->setObjectName ("ActionMarkItemAsRead_");

		Impl_->ActionMarkItemAsImportant_ = new QAction (tr ("Important"), this);
		Impl_->ActionMarkItemAsImportant_->setObjectName ("ActionMarkItemAsImportant_");
		Impl_->ActionMarkItemAsImportant_->setProperty ("ActionIcon", "rating");
		Impl_->ActionMarkItemAsImportant_->setCheckable (true);

		Impl_->ActionDeleteItem_ = new QAction (tr ("Delete"), this);
		Impl_->ActionDeleteItem_->setObjectName ("ActionDeleteItem_");
		Impl_->ActionDeleteItem_->setProperty ("ActionName", "remove");

		Impl_->ActionItemCommentsSubscribe_ = new QAction (tr ("Subscribe to comments"),
				this);
		Impl_->ActionItemCommentsSubscribe_->setObjectName ("ActionItemCommentsSubscribe_");

		Impl_->ActionItemLinkOpen_ = new QAction (tr ("Open in new tab"),
				this);
		Impl_->ActionItemLinkOpen_->setObjectName ("ActionItemLinkOpen_");
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
		int r, g, b;
		QApplication::palette ().color (group, role).getRgb (&r, &g, &b);
		long color = b + (g << 8) + (r << 16);
		QString result ("#%1");
		// Fill spare space with zeros.
		return result.arg (color, 6, 16, QChar ('0'));
	}

	QString ItemsWidget::ToHtml (const Item_ptr& item)
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
				item->Link_ +
				"'");
		if (linw)
			result += " target='_blank'";
		result += QString (">");
		result += (QString ("<strong>") +
				item->Title_ +
				"</strong>" +
				"</a><br />");

		// Publication date and author
		if (item->PubDate_.isValid () && !item->Author_.isEmpty ())
			result += tr ("Published on %1 by %2")
					.arg (item->PubDate_.toString ())
					.arg (item->Author_) +
				"<br />";
		else if (item->PubDate_.isValid ())
			result += tr ("Published on %1")
					.arg (item->PubDate_.toString ()) +
				"<br />";
		else if (!item->Author_.isEmpty ())
			result += tr ("Published by %1")
					.arg (item->Author_) +
				"<br />";

		// Categories
		if (item->Categories_.size ())
			result += item->Categories_.join ("; ") +
				"<br />";

		// Comments stuff
		if (item->NumComments_ >= 0 && !item->CommentsPageLink_.isEmpty ())
			result += tr ("%n comment(s), <a href='%1'%2>view them</a><br />",
					"", item->NumComments_)
					.arg (item->CommentsPageLink_)
					.arg (linw ? " target='_blank'" : "");
		else if (item->NumComments_ >= 0)
			result += tr ("%n comment(s)", "", item->NumComments_) +
				"<br />";
		else if (!item->CommentsPageLink_.isEmpty ())
			result += tr ("<a href='%1'%2>View comments</a><br />")
					.arg (item->CommentsPageLink_)
					.arg (linw ? " target='_blank'" : "");

		if (item->Latitude_ ||
				item->Longitude_)
		{
			QString link = QString ("http://maps.google.com/maps"
					"?f=q&source=s_q&hl=en&geocode=&q=%1+%2")
				.arg (item->Latitude_)
				.arg (item->Longitude_);
			result += tr ("Geoposition: <a href='%3'%4 title='Google Maps'>%1 %2</a><br />")
					.arg (item->Latitude_)
					.arg (item->Longitude_)
					.arg (link)
					.arg (linw ? " target='_blank'" : "");
		}

		// Description
		result += QString ("</div><div style='color: %1;"
				"padding-top: 0.5em; "
				"padding-left: 1em; "
				"padding-right: 1em;'>")
			.arg (GetHex (QPalette::Text));
		result += item->Description_;

		for (QList<Enclosure>::const_iterator i = item->Enclosures_.begin (),
				end = item->Enclosures_.end (); i != end; ++i)
		{
			result += inpad.arg (headerBg)
				.arg (headerText);
			if (i->Length_ > 0)
				result += tr ("File of type %1, size %2:<br />")
					.arg (i->Type_)
					.arg (LeechCraft::Util::MakePrettySize (i->Length_));
			else
				result += tr ("File of type %1 and unknown length:<br />")
					.arg (i->Type_);
			result += QString ("<a href='%1'>%2</a>")
				.arg (i->URL_)
				.arg (QFileInfo (QUrl (i->URL_).path ()).fileName ());
			if (!i->Lang_.isEmpty ())
				result += tr ("<br />Specified language: %1")
					.arg (i->Lang_);
			result += "</div>";
		}

		for (QList<MRSSEntry>::const_iterator entry = item->MRSSEntries_.begin (),
				endEntry = item->MRSSEntries_.end (); entry != endEntry; ++entry)
		{
			result += inpad.arg (headerBg)
				.arg (headerText);

			QString url = entry->URL_;

			if (entry->Medium_ == "image")
				result += tr ("Image ");
			else if (entry->Medium_ == "audio")
				result += tr ("Audio ");
			else if (entry->Medium_ == "video")
				result += tr ("Video ");
			else if (entry->Medium_ == "document")
				result += tr ("Document ");
			else if (entry->Medium_ == "executable")
				result += tr ("Executable ");

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
			Q_FOREACH (MRSSPeerLink pl, entry->PeerLinks_)
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
			Q_FOREACH (MRSSThumbnail thumb, entry->Thumbnails_)
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
			Q_FOREACH (MRSSScene sc, entry->Scenes_)
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
			Q_FOREACH (MRSSComment cm, entry->Comments_)
				comments [cm.Type_] += QString ("<li>%1</li>")
					.arg (cm.Comment_);

			QStringList cmTypes = comments.keys ();
			Q_FOREACH (QString type, cmTypes)
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
			Q_FOREACH (MRSSCredit cr, entry->Credits_)
			{
				if (cr.Role_.isEmpty ())
					continue;
				credits += QString ("<li>%1: %2</li>")
					.arg (cr.Role_)
					.arg (cr.Who_);
			}

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

	QModelIndexList ItemsWidget::GetSelected () const
	{
		QModelIndexList result;
		Q_FOREACH (const QModelIndex& idx,
				Impl_->Ui_.Items_->selectionModel ()->selectedRows ())
		{
			const QModelIndex& mapped = Impl_->
					ItemsFilterModel_->mapToSource (idx);
			if (!mapped.isValid ())
				continue;

			result << mapped;
		}
		return result;
	}

	void ItemsWidget::handleItemDataUpdated (Item_ptr item, Channel_ptr channel)
	{
		if (Impl_->CurrentItemsModel_->GetCurrentChannel () == channel->ChannelID_)
			Impl_->CurrentItemsModel_->ItemDataUpdated (item);
		else
			Q_FOREACH (std::shared_ptr<ItemsListModel> m, Impl_->SupplementaryModels_)
				if (m->GetCurrentChannel () == channel->ChannelID_)
				{
					m->ItemDataUpdated (item);
					break;
				}
	}

	void ItemsWidget::invalidateMergeMode ()
	{
		if (Impl_->MergeMode_)
		{
			SetMergeMode (false);
			SetMergeMode (true);
		}
	}

	void ItemsWidget::channelChanged (const QModelIndex& mapped)
	{
		Impl_->Ui_.Items_->scrollToTop ();
		currentItemChanged ();

		if (!isVisible ())
			return;

		QStringList allCategories = Core::Instance ().GetCategories (mapped);
		Impl_->ItemsFilterModel_->categorySelectionChanged (allCategories);

		if (allCategories.size ())
		{
			Impl_->ItemCategorySelector_->SetPossibleSelections (allCategories);
			Impl_->ItemCategorySelector_->selectAll ();
			if (XmlSettingsManager::Instance ()->
					property ("ShowCategorySelector").toBool ())
			Impl_->ItemCategorySelector_->show ();
			RestoreSplitter ();
		}
		else
		{
			Impl_->ItemCategorySelector_->SetPossibleSelections (QStringList ());
			Impl_->ItemCategorySelector_->hide ();
		}
	}

	void ItemsWidget::on_ActionHideReadItems__triggered ()
	{
		bool hide = Impl_->ActionHideReadItems_->isChecked ();
		XmlSettingsManager::Instance ()->
			setProperty ("HideReadItems", hide);
		SetHideRead (hide);
	}

	void ItemsWidget::on_ActionShowAsTape__triggered ()
	{
		SetTapeMode (!Impl_->TapeMode_);
	}

	void ItemsWidget::on_ActionMarkItemAsUnread__triggered ()
	{
		Q_FOREACH (const QModelIndex& idx, GetSelected ())
			MarkItemReadStatus (idx, false);
	}

	void ItemsWidget::on_ActionMarkItemAsRead__triggered ()
	{
		Q_FOREACH (const QModelIndex& idx, GetSelected ())
			MarkItemReadStatus (idx, true);
	}

	void ItemsWidget::on_ActionMarkItemAsImportant__triggered ()
	{
		StorageBackend *sb = Core::Instance ().GetStorageBackend ();

		const bool mark = Impl_->ActionMarkItemAsImportant_->isChecked ();

		const ITagsManager::tag_id impId = "_important";

		Q_FOREACH (const QModelIndex& idx, GetSelected ())
		{
			const QModelIndex& mapped = Impl_->ItemLists_->mapToSource (idx);
			const ItemsListModel *model =
					static_cast<ItemsListModel*> (Impl_->ItemLists_->
							GetModelForRow (idx.row ())->data ());

			const IDType_t item = model->GetItem (mapped).ItemID_;

			QList<ITagsManager::tag_id> tags = sb->GetItemTags (item);
			if (mark && !tags.contains (impId))
				sb->SetItemTags (item, tags + QStringList (impId));
			else if (!mark && tags.removeAll (impId))
				sb->SetItemTags (item, tags);
		}
	}

	void ItemsWidget::on_ActionDeleteItem__triggered ()
	{
		QSet<IDType_t> ids;
		Q_FOREACH (const QModelIndex& idx, GetSelected ())
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

		const auto& models = Impl_->ItemLists_->GetAllModels ();
		std::for_each (models.begin (), models.end (),
				[&ids] (QAbstractItemModel *model)
					{ qobject_cast<ItemsListModel*> (model)->RemoveItems (ids); });

		StorageBackend *sb = Core::Instance ().GetStorageBackend ();
		Q_FOREACH (IDType_t id, ids)
			sb->RemoveItem (id);
	}

	void ItemsWidget::on_CaseSensitiveSearch__stateChanged (int state)
	{
		Impl_->ItemsFilterModel_->setFilterCaseSensitivity (state ?
				Qt::CaseSensitive : Qt::CaseInsensitive);
	}

	void ItemsWidget::on_ActionItemCommentsSubscribe__triggered ()
	{
		Q_FOREACH (const QModelIndex& idx, GetSelected ())
			SubscribeToComments (idx);
	}

	void ItemsWidget::on_ActionItemLinkOpen__triggered ()
	{
		Q_FOREACH (const QModelIndex& idx, GetSelected ())
		{
			Entity e = Util::MakeEntity (QUrl (GetItem (idx)->Link_),
					QString (),
					FromUserInitiated | OnlyHandle);
			QMetaObject::invokeMethod (&Core::Instance (),
					"gotEntity",
					Qt::QueuedConnection,
					Q_ARG (LeechCraft::Entity, e));
		}
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
			const int idx = GetItem (current)->ItemID_;
			const QList<ITagsManager::tag_id>& tags = Core::Instance ()
					.GetStorageBackend ()->GetItemTags (idx);
			Impl_->ActionMarkItemAsImportant_->setChecked (tags.contains ("_important"));
		}

		QString preHtml = "<html><head><title>News</title></head><body bgcolor=\"";
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
				Item_ptr item = GetItem (mapped);
				if (!i)
					base = item->Link_;

				html += ToHtml (item);
			}

			Impl_->Ui_.ItemView_->SetHtml (preHtml + html + "</body></html>", base);
		}
		else
		{
			QString html;
			QUrl link;
			Q_FOREACH (const QModelIndex& selIndex,
					Impl_->Ui_.Items_->selectionModel ()->selectedRows ())
			{
				const QModelIndex& sindex = Impl_->
						ItemsFilterModel_->mapToSource (selIndex);
				if (!sindex.isValid ())
					continue;

				const Item_ptr item = GetItem (sindex);
				html += ToHtml (item);
				if (!link.isValid ())
					link = item->Link_;
			}

			Impl_->Ui_.ItemView_->SetHtml (QString (), QUrl ());
			Impl_->Ui_.ItemView_->SetHtml (preHtml + html + "</body></html>", link);

			const QModelIndex& sourceIndex =
					Impl_->Ui_.Items_->currentIndex ();
			const QModelIndex& cIndex =
					Impl_->ItemsFilterModel_->mapToSource (sourceIndex);
			if (html.isEmpty ())
			{
				Impl_->ActionItemCommentsSubscribe_->setEnabled (false);
				Impl_->ActionMarkItemAsUnread_->setEnabled (false);
				Impl_->ActionMarkItemAsRead_->setEnabled (false);
				Impl_->ActionItemLinkOpen_->setEnabled (false);
			}
			else
			{
				Selected (cIndex);

				QString commentsRSS = GetItem (cIndex)->CommentsLink_;
				Impl_->ActionItemCommentsSubscribe_->setEnabled (!commentsRSS.isEmpty ());

				Impl_->ActionMarkItemAsUnread_->setEnabled (true);
				Impl_->ActionMarkItemAsRead_->setEnabled (true);
				Impl_->ActionItemLinkOpen_->setEnabled (true);
			}
		}
	}

	void ItemsWidget::checkSelected ()
	{
		const QModelIndex& sourceIndex =
				Impl_->Ui_.Items_->currentIndex ();
		const QModelIndex& cIndex =
				Impl_->ItemsFilterModel_->mapToSource (sourceIndex);
		if (cIndex != Impl_->LastSelectedIndex_)
			return;

		QModelIndex mapped = Impl_->ItemLists_->mapToSource (cIndex);
		static_cast<ItemsListModel*> (Impl_->ItemLists_->
				GetModelForRow (cIndex.row ())->data ())->Selected (mapped);
	}

	void ItemsWidget::makeCurrentItemVisible ()
	{
		QModelIndex item = Impl_->Ui_.Items_->selectionModel ()->currentIndex ();
		if (item.isValid ())
			Impl_->Ui_.Items_->scrollTo (item);
	}

	void ItemsWidget::updateItemsFilter ()
	{
		const int section = Impl_->Ui_.SearchType_->currentIndex ();
		if (section == 4)
		{
			StorageBackend *sb = Core::Instance ().GetStorageBackend ();
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
			Impl_->ItemCategorySelector_->selectAll ();
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
