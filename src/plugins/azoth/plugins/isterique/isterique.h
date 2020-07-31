/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ISTERIQUE_ISTERIQUE_H
#define PLUGINS_AZOTH_PLUGINS_ISTERIQUE_ISTERIQUE_H
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>

namespace LC
{
namespace Azoth
{
namespace Isterique
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 IHaveSettings)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Isterique")

		Util::XmlSettingsDialog_ptr SettingsDialog_;
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
		void hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
	};
}
}
}

#endif
