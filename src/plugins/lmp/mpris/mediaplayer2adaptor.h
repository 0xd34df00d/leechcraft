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

namespace LC::LMP::MPRIS
{
	class MediaPlayer2Adaptor: public QDBusAbstractAdaptor
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")
		Q_CLASSINFO("D-Bus Introspection", ""
	"  <interface name=\"org.mpris.MediaPlayer2\">\n"
	"    <method name=\"Raise\"/>\n"
	"    <method name=\"Quit\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanQuit\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanSetFullscreen\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"CanQuit\"/>\n"
	"    <property access=\"read\" type=\"b\" name=\"HasTrackList\"/>\n"
	"    <property access=\"read\" type=\"s\" name=\"Identity\"/>\n"
	"    <property access=\"read\" type=\"s\" name=\"DesktopEntry\"/>\n"
	"    <property access=\"read\" type=\"as\" name=\"SupportedUriSchemes\"/>\n"
	"    <property access=\"read\" type=\"as\" name=\"SupportedMimeTypes\"/>\n"
	"  </interface>\n"
			"")
	public:
		explicit MediaPlayer2Adaptor (QObject*);

	public:
		Q_PROPERTY (bool CanQuit READ GetCanQuit)
		bool GetCanQuit () const;

		Q_PROPERTY (bool CanSetFullscreen READ GetCanSetFullscreen)
		bool GetCanSetFullscreen () const;

		Q_PROPERTY (QString DesktopEntry READ GetDesktopEntry)
		QString GetDesktopEntry () const;

		Q_PROPERTY (bool HasTrackList READ GetHasTrackList)
		bool GetHasTrackList () const;

		Q_PROPERTY (QString Identity READ GetIdentity)
		QString GetIdentity () const;

		Q_PROPERTY (QStringList SupportedMimeTypes READ GetSupportedMimeTypes)
		QStringList GetSupportedMimeTypes () const;

		Q_PROPERTY (QStringList SupportedUriSchemes READ GetSupportedUriSchemes)
		QStringList GetSupportedUriSchemes () const;
	public slots:
		void Quit ();
		void Raise ();
	signals:
		void raiseRequested ();
	};
}
