/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QFlags>

namespace LC
{
namespace SecMan
{
	class IStoragePlugin
	{
	public:
		virtual ~IStoragePlugin () {}

		enum StorageType
		{
			STInsecure,
			STSecure
		};

		Q_DECLARE_FLAGS (StorageTypes, StorageType)

		virtual StorageTypes GetStorageTypes () const = 0;
		virtual QList<QByteArray> ListKeys (StorageType st = STInsecure) = 0;

		virtual void Save (const QByteArray& key,
				const QVariant& value,
				StorageType st = STInsecure) = 0;
		virtual QVariant Load (const QByteArray& key, StorageType st = STInsecure) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::SecMan::IStoragePlugin,
		"org.LeechCraft.SecMan.IStoragePlugin/1.0")
