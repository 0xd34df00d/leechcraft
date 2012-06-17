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
	class MediaPlayer2Adaptor: public QDBusAbstractAdaptor
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
// 		Q_CLASSINFO("D-Bus Introspection", ""
// 	"  <interface name=\"org.mpris.MediaPlayer2\">\n"
// 	"    <method name=\"Raise\"/>\n"
// 	"    <method name=\"Quit\"/>\n"
// 	"    <property access=\"read\" type=\"b\" name=\"CanQuit\"/>\n"
// 	"    <property access=\"read\" type=\"b\" name=\"CanSetFullscreen\"/>\n"
// 	"    <property access=\"read\" type=\"b\" name=\"CanQuit\"/>\n"
// 	"    <property access=\"read\" type=\"b\" name=\"HasTrackList\"/>\n"
// 	"    <property access=\"read\" type=\"s\" name=\"Identity\"/>\n"
// 	"    <property access=\"read\" type=\"s\" name=\"DesktopEntry\"/>\n"
// 	"    <property access=\"read\" type=\"as\" name=\"SupportedUriSchemes\"/>\n"
// 	"    <property access=\"read\" type=\"as\" name=\"SupportedMimeTypes\"/>\n"
// 	"  </interface>\n"
// 			"")

		PlayerTab *Tab_;
	public:
		MediaPlayer2Adaptor (PlayerTab*);
		virtual ~MediaPlayer2Adaptor();

	public: // PROPERTIES
		Q_PROPERTY(bool CanQuit READ GetCanQuit)
		bool GetCanQuit () const;

		Q_PROPERTY(bool CanSetFullscreen READ GetCanSetFullscreen)
		bool GetCanSetFullscreen () const;

		Q_PROPERTY(QString DesktopEntry READ GetDesktopEntry)
		QString GetDesktopEntry () const;

		Q_PROPERTY(bool HasTrackList READ GetHasTrackList)
		bool GetHasTrackList () const;

		Q_PROPERTY(QString Identity READ GetIdentity)
		QString GetIdentity () const;

		Q_PROPERTY(QStringList SupportedMimeTypes READ GetSupportedMimeTypes)
		QStringList GetSupportedMimeTypes () const;

		Q_PROPERTY(QStringList SupportedUriSchemes READ GetSupportedUriSchemes)
		QStringList GetSupportedUriSchemes () const;
	public slots:
		void Quit ();
		void Raise ();
	};
}
}
}
