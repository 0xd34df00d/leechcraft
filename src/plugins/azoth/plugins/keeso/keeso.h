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
#include <interfaces/core/ihookproxy.h>
#include <interfaces/azoth/iprovidecommands.h>

namespace LC
{
namespace Azoth
{
namespace Keeso
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public IProvideCommands
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LC::Azoth::IProvideCommands)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Keeso")

		StaticCommands_t Commands_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		StaticCommands_t GetStaticCommands (ICLEntry*);
	};
}
}
}
