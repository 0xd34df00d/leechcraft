/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/blasq/iservicesplugin.h>

namespace LC
{
namespace Blasq
{
namespace Spegnersi
{
	class FlickrService;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IServicesPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2 LC::Blasq::IServicesPlugin)

		LC_PLUGIN_METADATA ("org.LeechCraft.Blasq.Spegnersi")

		FlickrService *Service_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		QList<IService*> GetServices () const;
	};
}
}
}
