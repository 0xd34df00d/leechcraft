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
#include <QMetaType>
#include <QDBusObjectPath>
#include <QIcon>

class QDBusArgument;

class ICoreProxy;

typedef std::shared_ptr<ICoreProxy> ICoreProxy_ptr;

Q_DECLARE_METATYPE (QIcon)

QDBusArgument& operator<< (QDBusArgument&, const ICoreProxy_ptr&);
const QDBusArgument& operator>> (const QDBusArgument&, ICoreProxy_ptr&);

QDBusArgument& operator<< (QDBusArgument&, const QIcon&);
const QDBusArgument& operator>> (const QDBusArgument&, QIcon&);

namespace LC
{
namespace DBus
{
	void RegisterTypes ();

	class ObjectManager : public QObject
	{
		Q_OBJECT

		quint64 Counter_;
	public:
		struct ObjectDataInfo
		{
			QString Service_;
			QDBusObjectPath Path_;
		};
	private:
		QHash<QObject*, ObjectDataInfo> Registered_;

		ObjectManager ();

		ObjectManager (const ObjectManager&) = delete;
		ObjectManager (ObjectManager&&) = delete;
	public:
		static ObjectManager& Instance ();

		template<typename T>
		ObjectDataInfo RegisterObject (std::shared_ptr<T>);

		template<typename T>
		ObjectDataInfo RegisterObject (T*);

		template<typename T>
		void Wrap (std::shared_ptr<T>&, const ObjectDataInfo&);

		template<typename T>
		void Wrap (T*&, const ObjectDataInfo&);
	private slots:
		void handleObjectDestroyed (QObject*);
	};
}
}
