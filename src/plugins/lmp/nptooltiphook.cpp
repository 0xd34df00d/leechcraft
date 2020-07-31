/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nptooltiphook.h"
#include <QEvent>
#include <QPixmap>
#include <QIcon>
#include <QToolTip>
#include <QPixmap>
#include <QHelpEvent>
#include <QFile>
#include <QtDebug>
#include <util/util.h>
#include "nowplayingpixmaphandler.h"
#include "core.h"
#include "localcollection.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	NPTooltipHook::NPTooltipHook (NowPlayingPixmapHandler *handler, QObject *parent)
	: QObject { parent }
	, PxHandler_ { handler }
	{
		PxHandler_->AddSetter ([this] (const QPixmap&, const QString&) { Base64Px_.clear (); });
	}

	void NPTooltipHook::SetTrackInfo (const MediaInfo& info)
	{
		Info_ = info;
	}

	namespace
	{
		void SetStatistics (QString& str, const QString& path)
		{
			const auto& stats = Core::Instance ().GetLocalCollection ()->GetTrackStats (path);
			const auto& lastPlayStr = stats ?
					QObject::tr ("Last playback at %1")
						.arg (FormatDateTime (stats.LastPlay_)) :
					QString {};
			const auto& countStr = stats ?
					NPTooltipHook::tr ("Played %n time(s) since %1", 0, stats.Playcount_)
						.arg (FormatDateTime (stats.Added_)) :
					QString {};
			str.replace ("${PLAYBACKS}", countStr);
			str.replace ("${LASTPLAY}", lastPlayStr);
		}
	}

	bool NPTooltipHook::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () != QEvent::ToolTip)
			return false;

		if (Base64Px_.isEmpty ())
		{
			const auto maxDim = 384;

			QImage img { PxHandler_->GetLastCoverPath () };
			if (img.isNull ())
				img = QIcon::fromTheme ("media-optical").pixmap (maxDim, maxDim).toImage ();

			if (img.width () > maxDim)
				img = img.scaled (maxDim, maxDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			Base64Px_ = Util::GetAsBase64Src (img);
		}

		QFile file { ":/lmp/resources/templates/nptooltip.html" };
		if (!file.open (QIODevice::ReadOnly))
		{
			qCritical () << Q_FUNC_INFO
					<< "unable to open"
					<< file.fileName ()
					<< file.errorString ();
			return true;
		}

		auto str = QString::fromUtf8 (file.readAll ());
		str.replace ("${TITLE}", Info_.Title_);
		str.replace ("${ARTIST}", Info_.Artist_);
		str.replace ("${ALBUM}", Info_.Album_);
		str.replace ("${GENRE}", Info_.Genres_.join (" / "));
		SetStatistics (str, Info_.LocalPath_);
		str.replace ("${IMG}", Base64Px_);

		const auto he = static_cast<QHelpEvent*> (event);
		QToolTip::showText (he->globalPos (), str, static_cast<QWidget*> (obj));

		return true;
	}
}
}
