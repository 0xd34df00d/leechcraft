/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "collectionwidget.h"
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QMenu>
#include <util/gui/clearlineeditaddon.h>
#include <util/xpc/defaulthookproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "localcollection.h"
#include "palettefixerfilter.h"
#include "collectiondelegate.h"
#include "audiopropswidget.h"
#include "util.h"
#include "albumartmanagerdialog.h"
#include "collectionsmanager.h"
#include "hookinterconnector.h"
#include "player.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class CollectionFilterModel : public QSortFilterProxyModel
		{
		public:
			CollectionFilterModel (QObject *parent = nullptr)
			: QSortFilterProxyModel { parent }
			{
				setDynamicSortFilter (true);
				setRecursiveFilteringEnabled (true);
			}
		protected:
			bool filterAcceptsRow (int sourceRow, const QModelIndex& sourceParent) const
			{
				const auto& source = sourceModel ()->index (sourceRow, 0, sourceParent);
				if (source.data (LocalCollectionModel::Role::IsTrackIgnored).toBool ())
					return false;

				const auto type = source.data (LocalCollectionModel::Role::Node).toInt ();
				const bool isTrack = type == LocalCollectionModel::NodeType::Track;
				const auto childrenCount = sourceModel ()->rowCount (source);
				if (!isTrack)
					for (int i = 0; i < childrenCount; ++i)
						if (filterAcceptsRow (i, source))
							return true;

				const auto& pattern = filterRegExp ().pattern ();

				if (pattern.isEmpty () && !isTrack && childrenCount)
					return false;

				auto check = [&source, &pattern] (int role)
				{
					return source.data (role).toString ().contains (pattern, Qt::CaseInsensitive);
				};
				return check (Qt::DisplayRole) ||
						check (LocalCollectionModel::Role::ArtistName) ||
						check (LocalCollectionModel::Role::AlbumName) ||
						check (LocalCollectionModel::Role::TrackTitle) ||
						check (LocalCollectionModel::Role::AlbumYear);
			}
		};
	}

	CollectionWidget::CollectionWidget (QWidget *parent)
	: QWidget { parent }
	, Player_ { Core::Instance ().GetPlayer () }
	, CollectionFilterModel_ { new CollectionFilterModel { this } }
	{
		Ui_.setupUi (this);

		new Util::ClearLineEditAddon (GetProxyHolder (), Ui_.CollectionFilter_);
		new PaletteFixerFilter (Ui_.CollectionTree_);

		connect (Core::Instance ().GetLocalCollection (),
				&LocalCollection::scanStarted,
				Ui_.ScanProgress_,
				&QProgressBar::setMaximum);
		connect (Core::Instance ().GetLocalCollection (),
				&LocalCollection::scanProgressChanged,
				this,
				&CollectionWidget::HandleScanProgress);
		connect (Core::Instance ().GetLocalCollection (),
				&LocalCollection::scanFinished,
				Ui_.ScanProgress_,
				&QProgressBar::hide);
		Ui_.ScanProgress_->hide ();

		Ui_.CollectionTree_->setItemDelegate (new CollectionDelegate (Ui_.CollectionTree_));
		auto collMgr = Core::Instance ().GetCollectionsManager ();
		CollectionFilterModel_->setSourceModel (collMgr->GetModel ());
		Ui_.CollectionTree_->setModel (CollectionFilterModel_);

		connect (Ui_.CollectionTree_,
				&QTreeView::doubleClicked,
				this,
				&CollectionWidget::LoadFromCollection);

		connect (Ui_.CollectionFilter_,
				&QLineEdit::textChanged,
				CollectionFilterModel_,
				&QSortFilterProxyModel::setFilterFixedString);

		Core::Instance ().GetHookInterconnector ()->RegisterHookable (this);
	}

	void CollectionWidget::ShowCollectionTrackProps ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& info = index.data (LocalCollectionModel::Role::TrackPath).toString ();
		if (info.isEmpty ())
			return;

		AudioPropsWidget::MakeDialog ()->SetProps (info);
	}

	void CollectionWidget::ShowCollectionAlbumArt ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& path = index.data (LocalCollectionModel::Role::AlbumArt).toString ();
		if (path.isEmpty ())
			return;

		ShowAlbumArt (path, QCursor::pos ());
	}

	void CollectionWidget::ShowAlbumArtManager ()
	{
		auto aamgr = Core::Instance ().GetLocalCollection ()->GetAlbumArtManager ();

		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto albumId = index.data (LocalCollectionModel::Role::AlbumID).toInt ();
		const auto& album = index.data (LocalCollectionModel::Role::AlbumName).toString ();
		const auto& artist = index.data (LocalCollectionModel::Role::ArtistName).toString ();

		auto dia = new AlbumArtManagerDialog (albumId, artist, album, aamgr, this);
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();
	}

	void CollectionWidget::ShowInArtistBrowser ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& artist = index.data (LocalCollectionModel::Role::ArtistName).toString ();
		Core::Instance ().RequestArtistBrowser (artist);
	}

	namespace
	{
		template<typename T>
		QList<T> CollectFromModel (const QModelIndex& root, int role)
		{
			QList<T> result;

			const auto& var = root.data (role);
			if (!var.isNull ())
				result << var.value<T> ();

			auto model = root.model ();
			for (int i = 0; i < model->rowCount (root); ++i)
				result += CollectFromModel<T> (model->index (i, 0, root), role);

			return result;
		}
	}

	void CollectionWidget::HandleCollectionRemove ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& paths = CollectFromModel<QString> (index, LocalCollectionModel::Role::TrackPath);
		if (paths.isEmpty ())
			return;

		auto collection = Core::Instance ().GetLocalCollection ();
		for (const auto& path : paths)
			collection->IgnoreTrack (path);
	}

	void CollectionWidget::HandleCollectionDelete ()
	{
		const auto& index = Ui_.CollectionTree_->currentIndex ();
		const auto& paths = CollectFromModel<QString> (index, LocalCollectionModel::Role::TrackPath);
		if (paths.isEmpty ())
			return;

		auto response = QMessageBox::question (this,
				"LeechCraft",
				tr ("Are you sure you want to erase %n track(s)? This action cannot be undone.",
					0,
					paths.size ()),
					QMessageBox::Yes | QMessageBox::No);
		if (response != QMessageBox::Yes)
			return;

		for (const auto& path : paths)
			QFile::remove (path);
	}

	void CollectionWidget::LoadFromCollection ()
	{
		const auto& idxs = Ui_.CollectionTree_->selectionModel ()->selectedRows ();

		QModelIndexList mapped;
		for (const auto& src : idxs)
		{
			const auto& index = CollectionFilterModel_->mapToSource (src);
			if (index.isValid ())
				mapped << index;
		}

		Core::Instance ().GetCollectionsManager ()->Enqueue (mapped, Player_);
	}

	namespace
	{
		MediaInfo ColIndex2MediaInfo (const QModelIndex& index)
		{
			return
			{
				index.data (LocalCollectionModel::Role::TrackPath).toString (),
				index.data (LocalCollectionModel::Role::ArtistName).toString (),
				index.data (LocalCollectionModel::Role::AlbumName).toString (),
				index.data (LocalCollectionModel::Role::TrackTitle).toString (),
				index.data (LocalCollectionModel::Role::TrackGenres).toStringList (),
				index.data (LocalCollectionModel::Role::TrackLength).toInt (),
				index.data (LocalCollectionModel::Role::AlbumYear).toInt (),
				index.data (LocalCollectionModel::Role::TrackNumber).toInt ()
			};
		}
	}

	void CollectionWidget::on_CollectionTree__customContextMenuRequested (const QPoint& point)
	{
		const auto& index = Ui_.CollectionTree_->indexAt (point);
		if (!index.isValid ())
			return;

		const int nodeType = index.data (LocalCollectionModel::Role::Node).value<int> ();

		QMenu menu;

		auto addToPlaylist = menu.addAction (tr ("Add to playlist"), this, &CollectionWidget::LoadFromCollection);
		addToPlaylist->setProperty ("ActionIcon", "list-add");

		menu.addAction (tr ("Replace playlist"),
				[this]
				{
					Player_->clear ();
					LoadFromCollection ();
				});

		if (nodeType == LocalCollectionModel::NodeType::Track)
		{
			auto showTrackProps = menu.addAction (tr ("Show track properties"),
					this, &CollectionWidget::ShowCollectionTrackProps);
			showTrackProps->setProperty ("ActionIcon", "document-properties");
		}

		if (nodeType == LocalCollectionModel::NodeType::Album)
		{
			auto showAlbumArt = menu.addAction (tr ("Show album art"), this, &CollectionWidget::ShowCollectionAlbumArt);
			showAlbumArt->setProperty ("ActionIcon", "media-optical");

			menu.addAction (tr ("Album art manager..."), this, &CollectionWidget::ShowAlbumArtManager);
		}

		auto showInArtistBrowser = menu.addAction (tr ("Show in artist browser"),
				this, &CollectionWidget::ShowInArtistBrowser);
		showInArtistBrowser->setIcon (QIcon { "lcicons:/lmp/resources/images/lmp_artist_browser.svg" });

		menu.addSeparator ();

		auto remove = menu.addAction (tr ("Remove from collection..."), this, &CollectionWidget::HandleCollectionRemove);
		remove->setProperty ("ActionIcon", "list-remove");

		auto del = menu.addAction (tr ("Delete from disk..."), this, &CollectionWidget::HandleCollectionDelete);
		del->setProperty ("ActionIcon", "edit-delete");

		emit hookCollectionContextMenuRequested (std::make_shared<Util::DefaultHookProxy> (),
				&menu, ColIndex2MediaInfo (index));

		GetProxyHolder ()->GetIconThemeManager ()->ManageWidget (&menu);

		menu.exec (Ui_.CollectionTree_->viewport ()->mapToGlobal (point));
	}

	void CollectionWidget::HandleScanProgress (int progress)
	{
		if (progress >= Ui_.ScanProgress_->maximum ())
		{
			Ui_.ScanProgress_->hide ();
			return;
		}

		if (!Ui_.ScanProgress_->isVisible ())
			Ui_.ScanProgress_->show ();
		Ui_.ScanProgress_->setValue (progress);
	}
}
}
