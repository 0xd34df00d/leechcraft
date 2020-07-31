/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QtDBus>

class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

namespace LC
{
namespace LMP
{
class Player;

namespace MPRIS
{
	class FDOPropsAdaptor;

	class PlayerAdaptor: public QDBusAbstractAdaptor
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

		FDOPropsAdaptor *Props_;
		Player *Player_;
	public:
		PlayerAdaptor (FDOPropsAdaptor *fdo, Player*);
	public:
		Q_PROPERTY (bool CanControl READ GetCanControl)
		bool GetCanControl () const;

		Q_PROPERTY (bool CanGoNext READ GetCanGoNext)
		bool GetCanGoNext () const;

		Q_PROPERTY (bool CanGoPrevious READ GetCanGoPrevious)
		bool GetCanGoPrevious () const;

		Q_PROPERTY (bool CanPause READ GetCanPause)
		bool GetCanPause () const;

		Q_PROPERTY (bool CanPlay READ GetCanPlay)
		bool GetCanPlay () const;

		Q_PROPERTY (bool CanSeek READ GetCanSeek)
		bool GetCanSeek () const;

		Q_PROPERTY (QString LoopStatus READ GetLoopStatus WRITE SetLoopStatus)
		QString GetLoopStatus () const;
		void SetLoopStatus (const QString& value);

		Q_PROPERTY (double MaximumRate READ GetMaximumRate)
		double GetMaximumRate () const;

		Q_PROPERTY (QVariantMap Metadata READ GetMetadata)
		QVariantMap GetMetadata () const;

		Q_PROPERTY (double MinimumRate READ GetMinimumRate)
		double GetMinimumRate () const;

		Q_PROPERTY (QString PlaybackStatus READ GetPlaybackStatus)
		QString GetPlaybackStatus () const;

		Q_PROPERTY (qlonglong Position READ GetPosition)
		qlonglong GetPosition () const;

		Q_PROPERTY (double Rate READ GetRate WRITE SetRate)
		double GetRate () const;
		void SetRate (double value);

		Q_PROPERTY (bool Shuffle READ GetShuffle WRITE SetShuffle)
		bool GetShuffle () const;
		void SetShuffle (bool value);

		Q_PROPERTY (double Volume READ GetVolume WRITE SetVolume)
		double GetVolume () const;
		void SetVolume (double value);
	private:
		void Notify (const QString&);
	public slots:
		void Next ();
		void OpenUri (const QString& Uri);
		void Pause ();
		void Play ();
		void PlayPause ();
		void Previous ();
		void Seek (qlonglong Offset);
		void SetPosition (const QDBusObjectPath& TrackId, qlonglong Position);
		void Stop ();
	signals:
		void Seeked (qlonglong Position);
	};
}
}
}
