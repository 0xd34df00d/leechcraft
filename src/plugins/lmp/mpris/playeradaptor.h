/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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
class PlayerTab;

namespace MPRIS
{
	class FDOPropsAdaptor;

	class PlayerAdaptor: public QDBusAbstractAdaptor
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")
		Q_CLASSINFO("D-Bus Introspection", ""
	"  <interface name=\"org.mpris.MediaPlayer2.Player\">\n"
	"    <method name=\"Next\"/>\n"
	"    <method name=\"Previous\"/>\n"
	"    <method name=\"Pause\"/>\n"
	"    <method name=\"PlayPause\"/>\n"
	"    <method name=\"Stop\"/>\n"
	"    <method name=\"Play\"/>\n"
	"    <method name=\"Seek\">\n"
	"      <arg direction=\"in\" type=\"x\" name=\"Offset\"/>\n"
	"    </method>\n"
	"    <method name=\"SetPosition\">\n"
	"      <arg direction=\"in\" type=\"o\" name=\"TrackId\"/>\n"
	"      <arg direction=\"in\" type=\"x\" name=\"Position\"/>\n"
	"    </method>\n"
	"    <method name=\"OpenUri\">\n"
	"      <arg direction=\"in\" type=\"s\" name=\"Uri\"/>\n"
	"    </method>\n"
	"    <signal name=\"Seeked\">\n"
	"      <arg direction=\"pos\" type=\"x\" name=\"Position\"/>\n"
	"    </signal>\n"
	"    <property access=\"read\" type=\"s\" name=\"PlaybackStatus\"/>\n"
	"    <property access=\"readwrite\" type=\"s\" name=\"LoopStatus\"/>\n"
	"    <property access=\"readwrite\" type=\"d\" name=\"Rate\"/>\n"
	"    <property access=\"readwrite\" type=\"s\" name=\"Shuffle\"/>\n"
	"    <property access=\"read\" type=\"a{sv}\" name=\"Metadata\">\n"
	"      <annotation value=\"QVariantMap\" name=\"org.qtproject.QtDBus.QtTypeName\"/>\n"
	"    </property>\n"
	"    <property access=\"readwrite\" type=\"d\" name=\"Volume\"/>\n"
	"    <property access=\"read\" type=\"x\" name=\"Position\"/>\n"
	"    <property access=\"read\" type=\"d\" name=\"MinimumRate\"/>\n"
	"    <property access=\"read\" type=\"d\" name=\"MaximumRate\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanGoNext\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanGoPrevious\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanPlay\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanPause\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanSeek\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanControl\"/>\n"
	"  </interface>\n"
			"")

		FDOPropsAdaptor *Props_;
		PlayerTab *Tab_;
	public:
		PlayerAdaptor (FDOPropsAdaptor *fdo, PlayerTab *tab);
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
