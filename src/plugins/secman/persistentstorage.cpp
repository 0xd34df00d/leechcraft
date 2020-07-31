/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "persistentstorage.h"
#include <QVariant>
#include "core.h"

namespace LC
{
namespace SecMan
{
	PersistentStorage::PersistentStorage ()
	{
	}

	bool PersistentStorage::HasKey (const QByteArray& key)
	{
		return Core::Instance ().Load (key).isValid ();
	}

	QVariant PersistentStorage::Get (const QByteArray& key)
	{
		return Core::Instance ().Load (key);
	}

	void PersistentStorage::Set (const QByteArray& key, const QVariant& value)
	{
		Core::Instance ().Store (key, value);
	}
}
}
