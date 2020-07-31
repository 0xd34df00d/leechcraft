/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/structures.h>

namespace LC
{
namespace SecMan
{
	class Core : public QObject
	{
		Core ();

		QObjectList StoragePlugins_;
	public:
		static Core& Instance ();

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		QObjectList GetStoragePlugins () const;

		void Store (const QByteArray&, const QVariant&);
		QVariant Load (const QByteArray&);
	private:
		void AddStoragePlugin (QObject *object);

		QObject* GetStoragePlugin () const;
	};
}
}
