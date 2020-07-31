/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>

class QPixmap;
class QUrl;

namespace LC
{
namespace LMP
{
	struct MediaInfo;

	class NowPlayingPixmapHandler : public QObject
	{
		Q_OBJECT

		bool IsValidPixmap_;
		QString LastArtist_;
		QString LastCoverPath_;
	public:
		typedef std::function<void (QPixmap, QString)> PixmapSetter_f;
	private:
		QList<PixmapSetter_f> Setters_;
	public:
		NowPlayingPixmapHandler (QObject* = 0);

		void AddSetter (const PixmapSetter_f);

		void HandleSongChanged (const MediaInfo&, const QString&, const QPixmap&, bool);

		QString GetLastCoverPath () const;
	public slots:
		void handleGotArtistImage (const QString&, const QUrl&);
	private slots:
		void handleDownloadedImage ();
	};
}
}
