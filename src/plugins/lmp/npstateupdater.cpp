/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "npstateupdater.h"
#include <QLabel>
#include <util/xpc/util.h>
#include <util/sll/slotclosure.h>
#include <interfaces/core/ientitymanager.h>
#include "player.h"
#include "engine/sourceobject.h"
#include "xmlsettingsmanager.h"
#include "nowplayingwidget.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	NPStateUpdater::NPStateUpdater (QLabel *label,
			NowPlayingWidget *npWidget, Player *player, QObject* parent)
	: QObject { parent }
	, NPLabel_ { label }
	, NPWidget_ { npWidget }
	, Player_ { player }
	{
		connect (Player_,
				SIGNAL (songChanged (MediaInfo)),
				this,
				SLOT (update (MediaInfo)));
		connect (Player_,
				SIGNAL (songInfoUpdated (MediaInfo)),
				this,
				SLOT (update (MediaInfo)));
		connect (Player_->GetSourceObject (),
				SIGNAL (stateChanged (SourceState, SourceState)),
				this,
				SLOT (update (SourceState)));

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { IgnoreNextStop_ = true; },
			Player_,
			SIGNAL (aboutToStopInternally ()),
			this
		};
	}

	void NPStateUpdater::AddPixmapHandler (const PixmapHandler_f& handler)
	{
		PixmapHandlers_ << handler;
	}

	QString NPStateUpdater::BuildNotificationText (const MediaInfo& info) const
	{
		if (Player_->GetState () == SourceState::Stopped)
			return tr ("Playback is stopped.");

		const auto& title = info.Title_.isEmpty () ? tr ("unknown song") : info.Title_;
		const auto& album = info.Album_.isEmpty () ? tr ("unknown album") : info.Album_;
		const auto& track = info.Artist_.isEmpty () ? tr ("unknown artist") : info.Artist_;

		return tr ("Now playing: %1 from %2 by %3")
				.arg ("<em>" + title + "</em>")
				.arg ("<em>" + album + "</em>")
				.arg ("<em>" + track + "</em>");
	}

	void NPStateUpdater::EmitNotification (const QString& text, QPixmap notifyPx)
	{
		if (text == LastNotificationString_)
			return;

		ForceEmitNotification (text, notifyPx);
	}

	void NPStateUpdater::ForceEmitNotification (const QString& text, QPixmap notifyPx)
	{
		LastNotificationString_ = text;

		int width = notifyPx.width ();
		if (width > 200)
		{
			while (width > 200)
				width /= 2;
			notifyPx = notifyPx.scaledToWidth (width);
		}

		auto e = Util::MakeNotification ("LMP", text, Priority::Info);
		e.Additional_ ["NotificationPixmap"] = notifyPx;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	namespace
	{
		struct PixmapInfo
		{
			QPixmap PX_;
			QString CoverPath_;
		};

		PixmapInfo GetPixmap (const MediaInfo& info)
		{
			PixmapInfo pi;

			pi.CoverPath_ = FindAlbumArtPath (info.LocalPath_);
			if (!pi.CoverPath_.isEmpty ())
				pi.PX_ = QPixmap (pi.CoverPath_);

			if (pi.PX_.isNull ())
			{
				pi.PX_ = QIcon::fromTheme ("media-optical").pixmap (128, 128);
				pi.CoverPath_.clear ();
			}

			return pi;
		}
	}

	void NPStateUpdater::Update (MediaInfo info)
	{
		if (Player_->GetState () == SourceState::Stopped)
			info = MediaInfo {};

		const auto& pxInfo = GetPixmap (info);

		const auto& text = BuildNotificationText (info);
		NPLabel_->setText (text);

		NPWidget_->SetTrackInfo (info);

		for (const auto& pxHandler : PixmapHandlers_)
			pxHandler (info, pxInfo.CoverPath_, pxInfo.PX_);

		if (!text.isEmpty () &&
				XmlSettingsManager::Instance ().property ("EnableNotifications").toBool ())
			EmitNotification (text, pxInfo.PX_);
	}

	void NPStateUpdater::forceEmitNotification ()
	{
		const auto& info = Player_->GetCurrentMediaInfo ();
		const auto& pxInfo = GetPixmap (info);
		ForceEmitNotification (BuildNotificationText (Player_->GetCurrentMediaInfo ()), pxInfo.PX_);
	}

	void NPStateUpdater::update (SourceState newState)
	{
		if (IgnoreNextStop_ && newState == SourceState::Stopped)
		{
			IgnoreNextStop_ = false;
			return;
		}

		Update (Player_->GetCurrentMediaInfo ());
	}

	void NPStateUpdater::update (const MediaInfo& info)
	{
		if (Player_->GetState () == SourceState::Stopped)
			return;

		Update (info);
	}
}
}
