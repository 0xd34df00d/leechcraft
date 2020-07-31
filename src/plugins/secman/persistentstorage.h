/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Vadim Misbakh-Soloviev, Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/ipersistentstorageplugin.h>

namespace LC
{
namespace SecMan
{
	class PersistentStorage : public QObject
							, public IPersistentStorage
	{
		Q_OBJECT
		Q_INTERFACES (IPersistentStorage)
	public:
		PersistentStorage ();

		bool HasKey (const QByteArray& key);
		QVariant Get (const QByteArray& key);
		void Set (const QByteArray& key, const QVariant& value);
	};
}
}
