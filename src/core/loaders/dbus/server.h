/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

namespace LC
{
namespace Loaders
{
	class SOPluginLoader;
}

namespace DBus
{
	class Server : public QObject
	{
		Q_OBJECT
		Q_CLASSINFO ("D-Bus Interface", "org.LeechCraft.Control")

		std::shared_ptr<Loaders::SOPluginLoader> Loader_;
	public:
		Server ();

		Q_INVOKABLE bool Load (const QString& path);
		Q_INVOKABLE bool Unload (const QString& path);

		Q_INVOKABLE void SetLcIconsPaths (const QStringList&);
	};
}
}
