/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "marshalling.h"
#include <QDBusMetaType>
#include <QDBusConnection>
#include <QIcon>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>
#include "coreproxyserverwrapper.h"
#include "coreproxyproxy.h"

QDBusArgument& operator<< (QDBusArgument& arg,
		const LC::DBus::ObjectManager::ObjectDataInfo& info)
{
	arg.beginStructure ();
	arg << info.Path_ << info.Service_;
	arg.endStructure ();
	return arg;
}

const QDBusArgument& operator>> (const QDBusArgument& arg,
		LC::DBus::ObjectManager::ObjectDataInfo& info)
{
	arg.beginStructure ();
	arg >> info.Path_ >> info.Service_;
	arg.endStructure ();
	return arg;
}

QDBusArgument& operator<< (QDBusArgument& arg, const ICoreProxy_ptr& proxy)
{
	return arg << LC::DBus::ObjectManager::Instance ().RegisterObject (proxy);
}

const QDBusArgument& operator>> (const QDBusArgument& arg, ICoreProxy_ptr& proxy)
{
	LC::DBus::ObjectManager::ObjectDataInfo info;
	arg >> info;
	LC::DBus::ObjectManager::Instance ().Wrap (proxy, info);
	return arg;
}

QDBusArgument& operator<< (QDBusArgument& arg, const QIcon& icon)
{
	QByteArray ba;
	{
		QDataStream ostr (&ba, QIODevice::WriteOnly);
		ostr << icon;
	}
	arg.beginStructure ();
	arg << ba;
	arg.endStructure ();
	return arg;
}

const QDBusArgument& operator>> (const QDBusArgument& arg, QIcon& icon)
{
	arg.beginStructure ();
	QByteArray ba;
	arg >> ba;
	arg.endStructure ();

	{
		QDataStream istr (ba);
		istr >> icon;
	}

	return arg;
}

namespace LC
{
namespace DBus
{
	namespace
	{
		template<typename T>
		struct AdaptorCreator;

		template<>
		struct AdaptorCreator<ICoreProxy>
		{
			static void Create (ICoreProxy *w, QObject *wObj)
			{
				new CoreProxyServerWrapper (w, wObj);
			}
		};

		template<typename T>
		struct ProxyCreator;

		template<>
		struct ProxyCreator<ICoreProxy>
		{
			static ICoreProxy* Create (const ObjectManager::ObjectDataInfo& info)
			{
				return new CoreProxyProxy { info.Service_, info.Path_ };
			}
		};
	}

	void RegisterTypes ()
	{
		qDBusRegisterMetaType<ICoreProxy_ptr> ();
		qDBusRegisterMetaType<QIcon> ();
	}

	ObjectManager::ObjectManager ()
	: Counter_ (0)
	{
	}

	ObjectManager& ObjectManager::Instance ()
	{
		static ObjectManager m;
		return m;
	}

	template<typename T>
	auto ObjectManager::RegisterObject (std::shared_ptr<T> obj) -> ObjectDataInfo
	{
		if (!obj)
			return {};

		return RegisterObject (obj.get ());
	}

	template<typename T>
	auto ObjectManager::RegisterObject (T *t) -> ObjectDataInfo
	{
		const auto qobj = dynamic_cast<QObject*> (t);
		if (!qobj)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast the type to QObject*";
			return {};
		}

		if (Registered_.contains (qobj))
			return Registered_.value (qobj);

		connect (qobj,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleObjectDestroyed (QObject*)));

		AdaptorCreator<T>::Create (t, qobj);

		const QString path { "/org/LeechCraft/Object_" + QString::number (Counter_++) };
		QDBusConnection::sessionBus ().registerObject (path,
				qobj, QDBusConnection::ExportAllContents);

		ObjectDataInfo info
		{
			"org.LeechCraft.MainInstance",
			QDBusObjectPath { path }
		};

		Registered_ [qobj] = info;

		return info;
	}

	template<typename T>
	void ObjectManager::Wrap (std::shared_ptr<T>& obj, const ObjectDataInfo& info)
	{
		T *rawObj;
		Wrap (rawObj, info);
		obj.reset (rawObj);
	}

	template<typename T>
	void ObjectManager::Wrap (T*& obj, const ObjectDataInfo& info)
	{
		obj = ProxyCreator<T>::Create (info);
	}

	void ObjectManager::handleObjectDestroyed (QObject *obj)
	{
		Registered_.remove (obj);
	}
}
}
