/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QtGlobal>

class QString;
class QVariant;
class QObject;

template<typename, typename>
class QMap;

using QVariantMap = QMap<QString, QVariant>;

namespace LC
{
namespace Loaders
{
	class IPluginLoader
	{
	public:
		virtual ~IPluginLoader () {}

		virtual quint64 GetAPILevel () = 0;

		virtual bool Load () = 0;

		virtual bool Unload () = 0;

		virtual QObject* Instance () = 0;

		virtual bool IsLoaded () const = 0;

		virtual QString GetFileName () const = 0;

		virtual QString GetErrorString () const = 0;

		virtual QVariantMap GetManifest () const = 0;
	};

	qint64 GetLibAPILevel (const QString&);

	typedef std::shared_ptr<IPluginLoader> IPluginLoader_ptr;
}
}
