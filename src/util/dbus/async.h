/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDBusConnection>
#include <QDBusPendingReply>
#include <util/sll/typegetter.h>
#include "dbusconfig.h"

namespace LC::Util::DBus
{
	class TypedSignals;

	struct Endpoint
	{
		QString Service;
		QString Path;
		QString Interface;
		QDBusConnection Conn;

		UTIL_DBUS_API QDBusPendingCall GetProperty (const QString& property) const;
		UTIL_DBUS_API QDBusPendingReply<QVariantMap> GetAllProperties () const;

		template<typename... Args>
		QDBusPendingCall Call (const QString& method, Args&&... args) const
		{
			auto msg = QDBusMessage::createMethodCall (Service, Path, Interface, method);
			msg.setArguments ({ std::forward<Args> (args)... });
			return Conn.asyncCall (msg);
		}
	};

	UTIL_DBUS_API QDBusPendingCall StartService (const QDBusConnection& conn, const QString& name);

	class UTIL_DBUS_API TypedSignals : public QObject
	{
		Q_OBJECT

		Endpoint Endpoint_;

		QHash<QString, QList<std::function<void (const QDBusMessage&)>>> Handlers_;
	public:
		explicit TypedSignals (Endpoint endpoint, QObject *parent)
		: QObject { parent }
		, Endpoint_ { std::move (endpoint) }
		{
		}

		template<typename F>
		bool Connect (const QString& name, F&& handler)
		{
			auto& handlers = Handlers_ [name];
			if (handlers.isEmpty ())
			{
				const auto connected = Endpoint_.Conn.connect (Endpoint_.Service,
						Endpoint_.Path,
						Endpoint_.Interface,
						name,
						this,
						SLOT (invokeHandlers (QDBusMessage)));
				if (!connected)
					return false;
			}

			auto unwrapper = [this, name, handler = std::forward<F> (handler)] (const QDBusMessage& msg)
			{
				const auto& args = msg.arguments ();
				if (args.size () < static_cast<qsizetype> (ArgCount_v<F>))
				{
					qCritical () << "expected" << ArgCount_v<F> << "arguments, got" << args.size () << "for" << name;
					return;
				}

				[&, this]<size_t... Ixs> (std::index_sequence<Ixs...>)
				{
					const auto allConvertable = ([&]
					{
						if (!args [Ixs].template canConvert<ArgType_t<F, Ixs>> ())
						{
							qWarning () << Endpoint_.Service << Endpoint_.Path << Endpoint_.Interface << name
									<< "unable to convert" << args [Ixs] << "to" << typeid (ArgType_t<F, Ixs>).name ();
							return false;
						}
						return true;
					} () && ...);
					if (allConvertable)
						handler (args [Ixs].template value<ArgType_t<F, Ixs>> ()...);
				} (std::make_index_sequence<ArgCount_v<F>> {});
			};
			handlers << std::move (unwrapper);

			return true;
		}
	private slots:
		void invokeHandlers (const QDBusMessage& msg) const
		{
			const auto& name = msg.member ();
			const auto& handlers = Handlers_ [name];
			if (handlers.isEmpty ())
				qWarning () << "no handlers for" << name;
			for (const auto& handler : handlers)
				handler (msg);
		}
	};

	struct EndpointWithSignals : Endpoint
	{
		std::unique_ptr<TypedSignals> TypedSignals_ {};

		template<typename F>
		bool Connect (const QString& name, F&& handler)
		{
			if (!TypedSignals_)
				TypedSignals_ = std::make_unique<TypedSignals> (*this, nullptr);
			return TypedSignals_->Connect (name, std::forward<F> (handler));
		}

		template<std::derived_from<QObject> Obj, typename R, typename... Args>
		bool Connect (const QString& name, Obj *ctx, R (Obj::*handler) (Args...))
		{
			return Connect (name, [ctx, handler] (Args... args) { std::invoke (handler, ctx, args...); });
		}
	};
}
