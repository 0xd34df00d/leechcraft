/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QIcon>
#include <QMap>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>

namespace LC
{
namespace Poshuku
{
namespace Keywords
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Poshuku.Keywords")

		ICoreProxy_ptr CoreProxy_;
		Util::XmlSettingsDialog_ptr SettingsDialog_;
		QMap<QString, QString> Keywords2Urls_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QSet<QByteArray> GetPluginClasses () const;
		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
		void UpdateKeywords (const QString& keyword, const QString& url);
		void RemoveKeyword (const QString& keyword);
	public slots:
		void hookURLEditReturnPressed (LC::IHookProxy_ptr proxy,
				QObject *browserWidget);
	};
}
}
}
