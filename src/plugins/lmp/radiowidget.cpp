/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiowidget.h"
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QInputDialog>
#include <QDir>
#include <QtDebug>
#include <util/gui/clearlineeditaddon.h>
#include <util/sll/functional.h>
#include <util/sll/prelude.h>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "player.h"
#include "previewhandler.h"
#include "radiomanager.h"
#include "engine/sourceobject.h"
#include "radiocustomdialog.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class StationsFilterModel : public QSortFilterProxyModel
		{
		public:
			StationsFilterModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
			}
		protected:
			bool filterAcceptsRow (int row, const QModelIndex& parent) const
			{
				const auto& pat = this->filterRegExp ().pattern ();
				if (pat.isEmpty ())
					return true;

				const auto& idx = sourceModel ()->index (row, 0, parent);
				if (idx.data (Media::RadioItemRole::ItemType).toInt () == Media::RadioType::None)
					return true;

				return idx.data ().toString ().contains (pat, Qt::CaseInsensitive);
			}
		};
	}

	RadioWidget::RadioWidget (QWidget *parent)
	: QWidget (parent)
	, StationsProxy_ (new StationsFilterModel (this))
	{
		Ui_.setupUi (this);

		StationsProxy_->setDynamicSortFilter (true);
		StationsProxy_->setSourceModel (Core::Instance ().GetRadioManager ()->GetModel ());
		Ui_.StationsView_->setModel (StationsProxy_);

		connect (Ui_.StationsSearch_,
				SIGNAL (textChanged (QString)),
				StationsProxy_,
				SLOT (setFilterFixedString (QString)));

		new Util::ClearLineEditAddon (GetProxyHolder (), Ui_.StationsSearch_);
	}

	void RadioWidget::SetPlayer (Player *player)
	{
		Player_ = player;
	}

	void RadioWidget::AddUrl (const QUrl& url)
	{
		RadioCustomDialog dia (this);
		dia.SetUrl (url);
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& unmapped = Ui_.StationsView_->currentIndex ();
		const auto& index = StationsProxy_->mapToSource (unmapped);
		Core::Instance ().GetRadioManager ()->
				AddUrl (index, dia.GetUrl (), dia.GetName ());
	}

	void RadioWidget::handleRefresh ()
	{
		const auto& unmapped = Ui_.StationsView_->currentIndex ();
		const auto& index = StationsProxy_->mapToSource (unmapped);
		Core::Instance ().GetRadioManager ()->Refresh (index);
	}

	void RadioWidget::handleAddUrl ()
	{
		AddUrl ({});
	}

	void RadioWidget::handleAddCurrentUrl ()
	{
		const auto& url = Player_->GetSourceObject ()->
				GetCurrentSource ().ToUrl ();
		if (url.isLocalFile ())
			return;

		AddUrl (url);
	}

	void RadioWidget::handleRemoveUrl ()
	{
		const auto& unmapped = Ui_.StationsView_->currentIndex ();
		const auto& index = StationsProxy_->mapToSource (unmapped);
		Core::Instance ().GetRadioManager ()->RemoveUrl (index);
	}

	void RadioWidget::handleDownloadTracks ()
	{
		const auto& indices = Util::Map (Ui_.StationsView_->selectionModel ()->selectedRows (),
				Util::BindMemFn (&QAbstractProxyModel::mapToSource, StationsProxy_));

		const auto radioMgr = Core::Instance ().GetRadioManager ();
		const auto& urlInfos = Util::Filter (radioMgr->GetSources (indices),
				[] (const Media::AudioInfo& info)
					{ return info.Other_ ["URL"].toUrl ().isValid (); });

		GrabTracks (urlInfos, this);
	}

	void RadioWidget::on_StationsView__customContextMenuRequested (const QPoint& point)
	{
		const auto& idx = Ui_.StationsView_->indexAt (point);
		if (!idx.isValid ())
			return;

		const auto type = idx.data (Media::RadioItemRole::ItemType).toInt ();
		const auto parentType = idx.parent ().data (Media::RadioItemRole::ItemType).toInt ();

		const auto iconsMgr = GetProxyHolder ()->GetIconThemeManager ();

		QMenu menu;
		menu.addAction (iconsMgr->GetIcon ("view-refresh"),
				tr ("Refresh"),
				this,
				SLOT (handleRefresh ()));

		switch (type)
		{
		case Media::RadioType::CustomAddableStreams:
		{
			menu.addAction (iconsMgr->GetIcon ("list-add"),
					tr ("Add an URL..."),
					this,
					SLOT (handleAddUrl ()));

			const auto& url = Player_->GetSourceObject ()->GetCurrentSource ().ToUrl ();
			if (url.isValid () && !url.isLocalFile ())
				menu.addAction (tr ("Add current stream..."),
						this,
						SLOT (handleAddCurrentUrl ()));
			break;
		}
		case Media::RadioType::TracksList:
		case Media::RadioType::TracksRoot:
		case Media::RadioType::SingleTrack:
		{
			menu.addAction (iconsMgr->GetIcon ("download"),
					tr ("Download tracks..."),
					this,
					SLOT (handleDownloadTracks ()));
			break;
		}
		default:
			break;
		}

		if (parentType == Media::RadioType::CustomAddableStreams)
		{
			menu.addAction (iconsMgr->GetIcon ("list-remove"),
					tr ("Remove this URL"),
					this,
					SLOT (handleRemoveUrl ()));
		}

		menu.exec (Ui_.StationsView_->viewport ()->mapToGlobal (point));
	}

	void RadioWidget::on_StationsView__doubleClicked (const QModelIndex& unmapped)
	{
		const auto& index = StationsProxy_->mapToSource (unmapped);
		Core::Instance ().GetRadioManager ()->Handle (index, Player_);
	}
}
}
