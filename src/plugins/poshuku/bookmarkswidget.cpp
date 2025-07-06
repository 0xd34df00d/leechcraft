/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "bookmarkswidget.h"
#include <QMessageBox>
#include <util/models/flattofoldersproxymodel.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "core.h"
#include "favoritesdelegate.h"
#include "favoritesmodel.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Poshuku
{
	BookmarksWidget::BookmarksWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		FavoritesFilterModel_ = std::make_unique<FilterModel> (this);
		FavoritesFilterModel_->setSourceModel (Core::Instance ().GetFavoritesModel ());

		const auto itm = Core::Instance ().GetProxy ()->GetTagsManager ();
		FlatToFolders_ = std::make_shared<Util::FlatToFoldersProxyModel> (itm);
		handleGroupBookmarks ();
		XmlSettingsManager::Instance ().RegisterObject ("GroupBookmarksByTags",
				this, "handleGroupBookmarks");

		Ui_.FavoritesView_->setItemDelegate (new FavoritesDelegate (this));
		Ui_.FavoritesView_->addAction (Ui_.ActionEditBookmark_);
		Ui_.FavoritesView_->addAction (Ui_.ActionDeleteBookmark_);
		connect (Ui_.FavoritesView_,
				SIGNAL (deleteSelected (const QModelIndex&)),
				this,
				SLOT (translateRemoveFavoritesItem (const QModelIndex&)));

		new Util::TagsCompleter (Ui_.FavoritesFilterLine_);
		Ui_.FavoritesFilterLine_->AddSelector ();
		connect (Ui_.FavoritesFilterLine_,
				SIGNAL (tagsChosen ()),
				this,
				SLOT (selectTagsMode ()));
		connect (Ui_.FavoritesFilterLine_,
				SIGNAL (textChanged (const QString&)),
				this,
				SLOT (updateFavoritesFilter ()));
		connect (Ui_.FavoritesFilterType_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (updateFavoritesFilter ()));
		connect (Ui_.FavoritesFilterCaseSensitivity_,
				SIGNAL (stateChanged (int)),
				this,
				SLOT (updateFavoritesFilter ()));

		const auto itemsHeader = Ui_.FavoritesView_->header ();
		const auto& fm = fontMetrics ();
		itemsHeader->resizeSection (0,
				fm.horizontalAdvance ("Average site title can be very big, it's also the "
					"most important part, so it's priority is the biggest."));
		itemsHeader->resizeSection (1,
				fm.horizontalAdvance ("Average URL could be very very long, but we don't account this."));
		itemsHeader->resizeSection (2,
				fm.horizontalAdvance ("Average tags list size should be like this."));
	}

	void BookmarksWidget::on_ActionEditBookmark__triggered ()
	{
		auto current = Ui_.FavoritesView_->selectionModel ()->currentIndex ();
		if (FlatToFolders_->GetSourceModel ())
			current = FlatToFolders_->MapToSource (current);
		if (!current.isValid ())
			return;

		Core::Instance ().GetFavoritesModel ()->EditBookmark (FavoritesFilterModel_->mapToSource (current));
	}

	void BookmarksWidget::on_ActionDeleteBookmark__triggered ()
	{
		QModelIndex current = Ui_.FavoritesView_->
			selectionModel ()->currentIndex ();
		if (!current.isValid ())
			return;

		translateRemoveFavoritesItem (current);
	}

	void BookmarksWidget::translateRemoveFavoritesItem (const QModelIndex& sourceIndex)
	{
		QModelIndex index = sourceIndex;
		if (FlatToFolders_->GetSourceModel ())
		{
			index = FlatToFolders_->MapToSource (index);
			if (!index.isValid ())
			{
				QList<QPersistentModelIndex> toDelete;
				for (int i = 0, rows = FlatToFolders_->rowCount (sourceIndex);
						i < rows; ++i)
				{
					index = FlatToFolders_->index (i, 0, sourceIndex);
					index = FlatToFolders_->MapToSource (index);
					index = FavoritesFilterModel_->mapToSource (index);
					toDelete << QPersistentModelIndex (index);
				}

				for (const auto& pIndex : toDelete)
					Core::Instance ().GetFavoritesModel ()->removeItem (pIndex);

				return;
			}
		}
		index = FavoritesFilterModel_->mapToSource (index);
		Core::Instance ().GetFavoritesModel ()->removeItem (index);
	}

	void BookmarksWidget::updateFavoritesFilter ()
	{
		int section = Ui_.FavoritesFilterType_->currentIndex ();
		const auto& text = Ui_.FavoritesFilterLine_->text ();

		switch (section)
		{
		case 1:
			FavoritesFilterModel_->SetTagsMode (true);
			FavoritesFilterModel_->setFilterFixedString (text);
			break;
		default:
			FavoritesFilterModel_->SetTagsMode (false);
			FavoritesFilterModel_->setFilterFixedString (text);
			break;
		}

		const bool isCs = Ui_.FavoritesFilterCaseSensitivity_->checkState () == Qt::Checked;
		FavoritesFilterModel_->setFilterCaseSensitivity (isCs ? Qt::CaseSensitive : Qt::CaseInsensitive);
	}

	void BookmarksWidget::on_FavoritesView__activated (const QModelIndex& act)
	{
		QModelIndex index;
		if (FlatToFolders_->GetSourceModel ())
			index = FlatToFolders_->MapToSource (act);

		Ui_.ActionEditBookmark_->setEnabled (index.isValid ());
		Ui_.ActionDeleteBookmark_->setEnabled (index.isValid ());

		if (index.isValid ())
			Core::Instance ().NewURL (index.sibling (index.row (),
						FavoritesModel::ColumnURL).data ().toString ());
		else if (act.isValid ())
			for (int i = 0, size = FlatToFolders_->rowCount (act);
					i < size; ++i)
			{
				QModelIndex idx = FlatToFolders_->
					index (i, FavoritesModel::ColumnURL, act);
				Core::Instance ().NewURL (FlatToFolders_->
						MapToSource (idx).data ().toString ());
			}
	}

	void BookmarksWidget::on_OpenInTabs__released ()
	{
		for (int i = 0, size = FavoritesFilterModel_->rowCount ();
				i < size; ++i)
			Core::Instance ().NewURL (FavoritesFilterModel_->index (i, FavoritesModel::ColumnURL).data ().toString ());
	}

	void BookmarksWidget::selectTagsMode ()
	{
		Ui_.FavoritesFilterType_->setCurrentIndex (3);
		updateFavoritesFilter ();
	}

	void BookmarksWidget::handleGroupBookmarks ()
	{
		if (XmlSettingsManager::Instance ().property ("GroupBookmarksByTags").toBool ())
		{
			FlatToFolders_->SetSourceModel (FavoritesFilterModel_.get ());
			Ui_.FavoritesView_->setModel (FlatToFolders_.get ());
		}
		else
		{
			FlatToFolders_->SetSourceModel (0);
			Ui_.FavoritesView_->setModel (FavoritesFilterModel_.get ());
		}
	}
}
}
