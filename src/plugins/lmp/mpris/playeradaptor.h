/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

namespace LeechCraft
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
		virtual ~PlayerAdaptor ();
	public: // PROPERTIES
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
	private slots:
		void handleSongChanged ();
		void handlePlayModeChanged ();
		void handleStateChanged ();
		void handleVolumeChanged ();
	signals:
		void Seeked (qlonglong Position);
	};
}
}
}
