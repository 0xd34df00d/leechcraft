/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QUrl>
#include <interfaces/media/iradiostation.h>
#include <interfaces/media/imodifiableradiostation.h>

namespace LC
{
namespace LMP
{
	class RadioCustomStreams;

	class RadioCustomStation : public QObject
							 , public Media::IRadioStation
							 , public Media::IModifiableRadioStation
	{
		Q_OBJECT
		Q_INTERFACES (Media::IRadioStation
				Media::IModifiableRadioStation)

		RadioCustomStreams * const RCS_;
		const QList<QUrl> TrackList_;
	public:
		RadioCustomStation (const QList<QUrl>&, RadioCustomStreams*);

		QObject* GetQObject ();
		void RequestNewStream ();
		QString GetRadioName () const;

		void AddItem (const QUrl&, const QString&);
		void RemoveItem (const QModelIndex&);
	signals:
		void gotNewStream (const QUrl&, const Media::AudioInfo&);
		void gotPlaylist (const QString&, const QString&);
		void gotAudioInfos (const QList<Media::AudioInfo>&);
		void gotError (const QString&);
	};
}
}
