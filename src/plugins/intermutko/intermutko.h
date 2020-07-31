/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <interfaces/iinfo.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>

namespace LC
{
namespace Intermutko
{
	class AcceptLangWidget;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Intermutko")

		Util::XmlSettingsDialog_ptr XSD_;

		AcceptLangWidget *AcceptLangWidget_ = nullptr;
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
	public slots:
		void hookNAMCreateRequest (LC::IHookProxy_ptr,
					QNetworkAccessManager*,
					QNetworkAccessManager::Operation*,
					QIODevice**);
	};
}
}
