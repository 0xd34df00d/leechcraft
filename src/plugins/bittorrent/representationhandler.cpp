/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "representationhandler.h"
#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <util/sll/qtutil.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "core.h"
#include "sessionsettingsmanager.h"
#include "tabwidget.h"

namespace LC::BitTorrent
{
	RepresentationHandler::RepresentationHandler ()
	: Actions_
	{
		{
			.Session_ = Core::Instance ()->GetSession (),
			.StatusKeeper_ = *Core::Instance ()->GetStatusKeeper (),
			.GetPreferredParent_ = [] { return GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow (); }
		}
	}
	, DownSelectorAction_ { Core::Instance ()->GetSessionSettingsManager (), &SessionSettingsManager::SetOverallDownloadRate, "Down"_qs, }
	, UpSelectorAction_ { Core::Instance ()->GetSessionSettingsManager (), &SessionSettingsManager::SetOverallUploadRate, "Up"_qs, }
	{
		auto toolbar = Actions_.GetToolbar ();
		auto openInTorrentTab = toolbar->addAction (tr ("Open in torrent tab"), this,
				[this]
				{
					if (CurrentRow_.isValid ())
						emit torrentTabFocusRequested (CurrentRow_);
				});
		openInTorrentTab->setIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		toolbar->addSeparator ();
		toolbar->addAction (openInTorrentTab);
		toolbar->addSeparator ();

		toolbar->addAction (&DownSelectorAction_);
		toolbar->addAction (&UpSelectorAction_);

		Menu_->addSeparator ();
		Menu_->addAction (openInTorrentTab);
	}

	RepresentationHandler::~RepresentationHandler () = default;

	QAbstractItemModel& RepresentationHandler::GetRepresentation ()
	{
		return *Core::Instance ();
	}

	void RepresentationHandler::HandleCurrentRowChanged (const QModelIndex& index)
	{
		CurrentRow_ = index;

		Actions_.SetCurrentIndex (index);
		if (TabWidget_)
			TabWidget_->SetCurrentTorrent (index);
	}

	void RepresentationHandler::HandleSelectedRowsChanged (const QModelIndexList& indexes)
	{
		Actions_.SetCurrentSelection (indexes);
	}

	QWidget* RepresentationHandler::GetInfoWidget ()
	{
		EnsureTabWidgetCreated ();
		return &*TabWidget_;
	}

	QToolBar* RepresentationHandler::GetControls ()
	{
		return Actions_.GetToolbar ();
	}

	QMenu* RepresentationHandler::GetContextMenu ()
	{
		return Menu_.get ();
	}

	void RepresentationHandler::UpdateSpeedControllerOptions ()
	{
		DownSelectorAction_.HandleSpeedsChanged ();
		UpSelectorAction_.HandleSpeedsChanged ();
	}

	void RepresentationHandler::EnsureTabWidgetCreated ()
	{
		if (!TabWidget_)
		{
			const auto core = Core::Instance ();
			TabWidget_ = std::make_unique<TabWidget> (*core, core->GetSession (), *core->GetSessionSettingsManager ());
			TabWidget_->SetCurrentTorrent (CurrentRow_);

			UpdateTimer_.callOnTimeout (&*TabWidget_, &TabWidget::UpdateTorrentStats);
			UpdateTimer_.start (2000);
		}
	}
}
