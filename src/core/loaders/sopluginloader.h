/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "ipluginloader.h"

class QPluginLoader;

namespace LC
{
namespace Loaders
{
	class SOPluginLoader : public IPluginLoader
	{
		std::shared_ptr<QPluginLoader> Loader_;
		bool IsLoaded_ = false;
	public:
		SOPluginLoader (const QString&);

		SOPluginLoader (const SOPluginLoader&) = delete;
		SOPluginLoader (SOPluginLoader&&) = delete;
		SOPluginLoader& operator= (const SOPluginLoader&) = delete;
		SOPluginLoader& operator= (SOPluginLoader&&) = delete;

		quint64 GetAPILevel ();

		bool Load ();
		bool Unload ();

		QObject* Instance ();
		bool IsLoaded () const;
		QString GetFileName () const;
		QString GetErrorString () const;
		QVariantMap GetManifest () const;
	};
}
}
