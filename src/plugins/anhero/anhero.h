/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <interfaces/iinfo.h>
#include <interfaces/ishutdownlistener.h>

namespace LC
{
namespace AnHero
{
	class Plugin : public QObject
				 , public IInfo
				 , public IShutdownListener
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IShutdownListener)

		LC_PLUGIN_METADATA ("org.LeechCraft.AnHero")
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		void HandleShutdownInitiated ();
	};
}
}
