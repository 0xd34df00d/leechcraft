/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "favoritesmodel.h"
#include <algorithm>
#include <QTimer>
#include <QtDebug>
#include <QMimeData>
#include <QFileInfo>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <util/models/modelsanitizer.h>
#include <util/sll/prelude.h>
#include <util/sll/views.h>
#include <util/xpc/defaulthookproxy.h>
#include "core.h"
#include "editbookmarkdialog.h"

namespace LC
{
namespace Poshuku
{
	FavoritesModel::FavoritesModel (QObject *parent)
	: QAbstractItemModel (parent)
	{
		ItemHeaders_ << tr ("Title")
			<< tr ("URL")
			<< tr ("Tags");

		Util::InstallModelSanitizer (*this);
	}

	void FavoritesModel::HandleStorageReady ()
	{
		loadData ();
	}

	int FavoritesModel::columnCount (const QModelIndex&) const
	{
		return ItemHeaders_.size ();
	}

	QVariant FavoritesModel::data (const QModelIndex& index, int role) const
	{
		if (!index.isValid ())
			return QVariant ();

		switch (role)
		{
		case Qt::DisplayRole:
			switch (index.column ())
			{
			case ColumnTitle:
				return Items_ [index.row ()].Title_;
			case ColumnURL:
				return Items_ [index.row ()].URL_;
			case ColumnTags:
				return Core::Instance ().GetProxy ()->GetTagsManager ()->Join (GetVisibleTags (index.row ()));
			default:
				return {};
			}
		case Qt::DecorationRole:
			if (index.column () == ColumnTitle)
				return Core::Instance ().GetIcon (Items_ [index.row ()].URL_);
			else
				return QVariant ();
		case Qt::ToolTipRole:
			return CheckResults_ [Items_ [index.row ()].URL_];
		case +CustomDataRoles::Tags:
			return Items_ [index.row ()].Tags_;
		default:
			return {};
		}
	}

	Qt::ItemFlags FavoritesModel::flags (const QModelIndex& index) const
	{
		if (!index.isValid ())
			return Qt::ItemIsDropEnabled;

		Qt::ItemFlags result = Qt::ItemIsEnabled |
				Qt::ItemIsSelectable |
				Qt::ItemIsDragEnabled |
				Qt::ItemIsDropEnabled;
		if (index.column () == ColumnTags)
			result |= Qt::ItemIsEditable;
		return result;
	}

	QVariant FavoritesModel::headerData (int column, Qt::Orientation orient,
			int role) const
	{
		if (orient == Qt::Horizontal && role == Qt::DisplayRole)
			return ItemHeaders_.at (column);
		else
			return QVariant ();
	}

	QModelIndex FavoritesModel::index (int row, int column,
			const QModelIndex& parent) const
	{
		if (!hasIndex (row, column, parent))
			return QModelIndex ();

		return createIndex (row, column);
	}

	QModelIndex FavoritesModel::parent (const QModelIndex&) const
	{
		return QModelIndex ();
	}

	int FavoritesModel::rowCount (const QModelIndex& index) const
	{
		return index.isValid () ? 0 : Items_.size ();
	}

	bool FavoritesModel::setData (const QModelIndex& index,
			const QVariant& value, int)
	{
		if (!index.isValid () || index.row () >= Items_.size ())
			return false;

		auto item = Items_ [index.row ()];
		switch (index.column ())
		{
		case ColumnTags:
			item.Tags_ = Core::Instance ().GetProxy ()->GetTagsManager ()->GetIDs (value.toStringList ());
			break;
		case ColumnTitle:
			item.Title_ = value.toString ();
			break;
		default:
			return false;
		}

		try
		{
			// The storage's `updated` signal applies the change to Items_ and emits dataChanged.
			Core::Instance ().GetStorageBackend ()->UpdateFavorites (item);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			return false;
		}

		return true;
	}

	QModelIndex FavoritesModel::addItem (const QString& title,
			const QString& url, const QStringList& visibleTags)
	{
		if (url.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO << "refusing to add a favorite without a URL:" << title;
			return {};
		}

		if (const auto pos = FindItem (url); pos != Items_.end ())
			return index (std::distance (Items_.begin (), pos), 0);

		const auto& tags = Core::Instance ().GetProxy ()->GetTagsManager ()->GetIDs (visibleTags);

		try
		{
			// The storage's `added` signal appends the item to Items_.
			Core::Instance ().GetStorageBackend ()->AddToFavorites ({ title, url, tags });
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO << e.what ();
			return {};
		}

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookAddedToFavorites (proxy, title, url, visibleTags);

		const auto pos = FindItem (url);
		return pos == Items_.end () ? QModelIndex {} : index (std::distance (Items_.begin (), pos), 0);
	}

	QList<QVariant> FavoritesModel::getItemsMap() const
	{
		const auto itm = Core::Instance ().GetProxy ()->GetTagsManager ();
		return Util::Map (Items_,
				[itm] (const auto& item) -> QVariant
				{
					return QVariantMap
					{
						{ "Title", item.Title_ },
						{ "URL", item.URL_ },
						{ "Tags", itm->GetTags (item.Tags_) }
					};
				});
	}

	Qt::DropActions FavoritesModel::supportedDropActions () const
	{
		return static_cast<Qt::DropActions> (Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
	}

	QStringList FavoritesModel::mimeTypes () const
	{
		return { "text/uri-list" };
	}

	QMimeData* FavoritesModel::mimeData (const QModelIndexList& indexes) const
	{
		if (indexes.isEmpty ())
			return 0;

		QList<QUrl> urls;
		QStringList texts;
		QList<int> rows;
		for (const auto& index : indexes)
			if (!rows.contains (index.row ()))
				rows << index.row ();
		for (const auto& row : rows)
		{
			const auto& item = Items_ [row];
			urls << QUrl (item.URL_);
			texts << item.Title_;
		}

		auto data = new QMimeData ();
		data->setUrls (urls);
		data->setText (texts.join (";"));
		return data;
	}

	bool FavoritesModel::dropMimeData (const QMimeData *data, Qt::DropAction, int, int, const QModelIndex&)
	{
		const auto& urls = data->urls ();

		QStringList visibleTags;
		if (data->hasFormat ("x-leechcraft/tag"))
		{
			auto tm = Core::Instance ().GetProxy ()->GetTagsManager ();
			const auto& visible = tm->GetTag (data->data ("x-leechcraft/tag"));
			if (!visible.isEmpty ())
				visibleTags << visible;
		}

		auto tryAddUrl = [&visibleTags, this] (const QString& title, const QUrl& url)
		{
			const auto& urlStr = url.toString ();
			const auto pos = FindItem (urlStr);
			if (pos == Items_.end ())
			{
				addItem (title, urlStr, visibleTags);
				return;
			}

			const auto row = std::distance (Items_.begin (), pos);
			auto tags = GetVisibleTags (row);
			tags += visibleTags;
			tags.removeDuplicates ();
			setData (index (row, ColumnTags), tags);
		};

		if (urls.size () == 1 && !data->text ().isEmpty ())
			tryAddUrl (data->text (), urls.first ());
		else if (!urls.isEmpty ())
		{
			auto texts = data->text ().split (';', Qt::SkipEmptyParts);
			if (texts.size () != urls.size ())
			{
				texts.clear ();
				for (const auto& url : urls)
					texts << QFileInfo (url.path ()).fileName ();
			}

			for (const auto& pair : Util::Views::Zip (texts, urls))
				tryAddUrl (pair.first, pair.second);
		}

		return true;
	}

	void FavoritesModel::EditBookmark (const QModelIndex& source)
	{
		if (!source.isValid ())
			return;

		const auto& currentURL = source.sibling (source.row (),
				FavoritesModel::ColumnURL).data ().toString ();

		EditBookmarkDialog dia (source);
		if (dia.exec () != QDialog::Accepted)
			return;

		setData (source.sibling (source.row (), FavoritesModel::ColumnTitle), dia.GetTitle ());
		setData (source.sibling (source.row (), FavoritesModel::ColumnTags), dia.GetTags ());

		if (currentURL != dia.GetURL ())
			ChangeURL (source, dia.GetURL ());
	}

	void FavoritesModel::ChangeURL (const QModelIndex& index,
			const QString& url)
	{
		FavoritesItem item = Items_.at (index.row ());
		if (item.URL_ == url)
			return;

		removeItem (index);
		item.URL_ = url;
		Core::Instance ().GetStorageBackend ()->AddToFavorites (item);
	}

	const FavoritesModel::items_t& FavoritesModel::GetItems () const
	{
		return Items_;
	}

	void FavoritesModel::SetCheckResults (const QMap<QString, QString>& res)
	{
		CheckResults_ = res;
	}

	bool FavoritesModel::IsUrlExists (const QString& url) const
	{
		return FindItem (url) != Items_.end ();
	}

	QStringList FavoritesModel::GetVisibleTags (int index) const
	{
		return Core::Instance ().GetProxy ()->GetTagsManager ()->GetTags (Items_ [index].Tags_);
	}

	void FavoritesModel::removeItem (const QModelIndex& index)
	{
		if (!index.isValid () || index.row () >= Items_.size ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid index"
					<< index
					<< Items_.size ();
			return;
		}

		// A copy: the storage's `removed` signal erases the element synchronously.
		const auto item = Items_ [index.row ()];
		Core::Instance ().GetStorageBackend ()->RemoveFromFavorites (item);
		Core::Instance ().RemoveFromFavorites (item.URL_);
	}

	void FavoritesModel::removeItem (const QString& url)
	{
		const auto pos = FindItem (url);
		if (pos == Items_.end ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no favorite with URL"
					<< url;
			return;
		}

		const auto item = *pos;
		Core::Instance ().GetStorageBackend ()->RemoveFromFavorites (item);
		Core::Instance ().RemoveFromFavorites (url);
	}

	void FavoritesModel::handleItemAdded (const FavoritesModel::FavoritesItem& item)
	{
		beginInsertRows (QModelIndex (), rowCount (), rowCount ());
		Items_.push_back (item);
		endInsertRows ();
	}

	void FavoritesModel::handleItemUpdated (const FavoritesModel::FavoritesItem& item)
	{
		const auto pos = FindItem (item.URL_);
		if (pos == Items_.end ())
		{
			qWarning () << Q_FUNC_INFO << "not found updated item";
			return;
		}

		*pos = item;

		const int n = std::distance (Items_.begin (), pos);
		emit dataChanged (index (n, 0), index (n, columnCount () - 1));
	}

	void FavoritesModel::handleItemRemoved (const FavoritesModel::FavoritesItem& item)
	{
		const auto pos = FindItem (item.URL_);
		if (pos == Items_.end ())
		{
			qWarning () << Q_FUNC_INFO << "not found removed item";
			return;
		}

		const int n = std::distance (Items_.begin (), pos);
		beginRemoveRows (QModelIndex (), n, n);
		Items_.erase (pos);
		endRemoveRows ();
	}

	void FavoritesModel::loadData ()
	{
		items_t items;
		Core::Instance ().GetStorageBackend ()->LoadFavorites (items);

		if (!items.size ())
			return;

		const auto first = Items_.size ();
		beginInsertRows (QModelIndex (), first, first + items.size () - 1);
		for (items_t::iterator i = items.begin (), end = items.end (); i != end; ++i)
		{
			for (const auto& tag : QStringList { i->Tags_ })
			{
				const auto& ut = Core::Instance ().GetProxy ()->GetTagsManager ()->GetTag (tag);
				if (ut.isEmpty ())
					i->Tags_.removeAll (tag);
			}

			Items_.push_back (*i);
		}
		endInsertRows ();
	}
}
}
