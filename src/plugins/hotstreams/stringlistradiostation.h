/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <interfaces/media/iradiostation.h>

namespace LC
{
namespace HotStreams
{
	class StringListRadioStation : public QObject
								 , public Media::IRadioStation
	{
		Q_OBJECT
		Q_INTERFACES (Media::IRadioStation)

		QString Name_;
		QList<QUrl> URLs_;
	public:
		StringListRadioStation (const QList<QUrl>&, const QString&);

		QObject* GetQObject ();
		QString GetRadioName () const;
		void RequestNewStream ();
	private slots:
		void emitPlaylist ();
	signals:
		void gotNewStream (const QUrl&, const Media::AudioInfo&);
		void gotPlaylist (const QString&, const QString&);
		void gotAudioInfos (const QList<Media::AudioInfo>& infos);
		void gotError (const QString&);
	};
}
}
