/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <atomic>
#include <QStringList>
#include <QFileInfo>
#include <interfaces/media/idiscographyprovider.h>
#include "interfaces/lmp/ilmpproxy.h"
#include "interfaces/lmp/ilmputilproxy.h"

class QPixmap;
class QPoint;
class QColor;

namespace LC
{
struct Entity;

namespace LMP
{
	struct MediaInfo;

	QList<QFileInfo> RecIterateInfo (const QString& dirPath,
			bool followSymlinks = false, std::atomic<bool> *stopFlag = nullptr);
	QStringList RecIterate (const QString& dirPath, bool followSymlinks = false);

	QString FindAlbumArtPath (const QString& near, bool ignoreCollection = false);

	template<typename T = QPixmap>
	T FindAlbumArt (const QString& nearPath, bool ignoreCollection = false)
	{
		if (nearPath.isEmpty ())
			return {};

		const T nearPx { nearPath };
		if (!nearPx.isNull ())
			return nearPx;

		return T { FindAlbumArtPath (nearPath, ignoreCollection) };
	}

	void ShowAlbumArt (const QString& near, const QPoint& pos);

	QString PerformSubstitutionsPlaylist (const MediaInfo& info);

	QString MakeTrackListTooltip (const QList<QList<Media::ReleaseTrackInfo>>&);

	bool CompareArtists (QString, QString, bool withoutThe);

	QPair<QString, QColor> GetRuleSymbol (const Entity&);

	QString FormatDateTime (const QDateTime& datetime);

	void GrabTracks (const QList<Media::AudioInfo>& infos, QWidget *parent = nullptr);
	void GrabTracks (const QList<MediaInfo>& infos, QWidget *parent = nullptr);
}
}
