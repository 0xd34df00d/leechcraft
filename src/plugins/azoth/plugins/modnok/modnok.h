/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QCache>
#include <QImage>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>

class QImage;

namespace LC
{
namespace Azoth
{
namespace Modnok
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Modnok")

		Util::XmlSettingsDialog_ptr SettingsDialog_;

		QString ConvScriptPath_;
		QCache<QString, QImage> FormulasCache_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	private:
		QImage GetRenderedImage (const QString&);
		std::optional<QString> NormalizeAndRender (QString);
		QString HandleBody (QString);
	public slots:
		void hookFormatBodyEnd (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookGonnaHandleSmiles (LC::IHookProxy_ptr proxy,
				QString body,
				QString pack);
		void hookMessageCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
	private slots:
		void clearCaches ();
		void handleCacheSize ();
	};
}
}
}
