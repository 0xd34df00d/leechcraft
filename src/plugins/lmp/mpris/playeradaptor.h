/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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
