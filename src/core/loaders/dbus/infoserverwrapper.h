/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QIcon>
#include <interfaces/iinfo.h>

namespace LC
{
namespace DBus
{
	class InfoServerWrapper : public QObject
							, public IInfo
	{
		Q_OBJECT
		Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.IInfo")

		IInfo * const W_;
	public:
		InfoServerWrapper (IInfo*);

		Q_INVOKABLE void SetProxy (ICoreProxy_ptr);
		Q_INVOKABLE void SetPluginInstance (QObject*);

		Q_INVOKABLE void Init (ICoreProxy_ptr);
		Q_INVOKABLE void SecondInit ();
		Q_INVOKABLE void Release ();

		Q_INVOKABLE QByteArray GetUniqueID () const;
		Q_INVOKABLE QString GetName () const;
		Q_INVOKABLE QString GetInfo () const;
		Q_INVOKABLE QIcon GetIcon () const;
	};
}
}
