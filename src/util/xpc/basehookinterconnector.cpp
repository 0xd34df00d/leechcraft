/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "basehookinterconnector.h"
#include <QMetaMethod>
#include <QtDebug>

namespace LC
{
namespace Util
{
	BaseHookInterconnector::BaseHookInterconnector (QObject *parent)
	: QObject (parent)
	{
	}

	BaseHookInterconnector::~BaseHookInterconnector ()
	{
	}

	namespace
	{
		bool IsHookMethod (const QMetaMethod& method)
		{
			return method.parameterTypes ().value (0) == "LC::IHookProxy_ptr";
		}

		auto BuildHookSlots (const QObject *obj)
		{
			const auto objMo = obj->metaObject ();

			QHash<QByteArray, QList<QMetaMethod>> hookSlots;
			for (int i = 0, size = objMo->methodCount (); i < size; ++i)
			{
				const auto& method = objMo->method (i);
				if (IsHookMethod (method))
					hookSlots [method.name ()] << method;
			}

			return hookSlots;
		}

		void CheckMatchingSigs (const QObject *snd, const QObject *rcv)
		{
			if (!qEnvironmentVariableIsSet ("LC_VERBOSE_HOOK_CHECKS"))
				return;

			const auto& hookSlots = BuildHookSlots (snd);

			const auto rcvMo = rcv->metaObject ();

			for (int i = 0, size = rcvMo->methodCount (); i < size; ++i)
			{
				const auto& rcvMethod = rcvMo->method (i);
				if (!IsHookMethod (rcvMethod))
					continue;

				const auto& rcvName = rcvMethod.name ();
				if (!hookSlots.contains (rcvName))
				{
					qWarning () << Q_FUNC_INFO
							<< "no method matching method"
							<< rcvName
							<< "(receiver"
							<< rcv
							<< ") in sender object"
							<< snd;
					continue;
				}

				const auto& sndMethods = hookSlots [rcvName];
				if (std::none_of (sndMethods.begin (), sndMethods.end (),
							[&rcvMethod] (const QMetaMethod& sndMethod)
							{
								return QMetaObject::checkConnectArgs (sndMethod, rcvMethod);
							}))
					qWarning () << Q_FUNC_INFO
							<< "incompatible signatures for hook"
							<< rcvName
							<< "in"
							<< snd
							<< "and"
							<< rcv;
			}
		}

#define LC_N(a) (QMetaObject::normalizedSignature(a))
#define LC_TOSLOT(a) ('1' + QByteArray(a))
#define LC_TOSIGNAL(a) ('2' + QByteArray(a))
		void ConnectHookSignals (QObject *sender, QObject *receiver, bool destSlot)
		{
			if (destSlot)
				CheckMatchingSigs (sender, receiver);

			const QMetaObject *mo = sender->metaObject ();
			for (int i = 0, size = mo->methodCount (); i < size; ++i)
			{
				QMetaMethod method = mo->method (i);
				if (method.methodType () != QMetaMethod::Signal)
					continue;

				if (!IsHookMethod (method))
					continue;

				const auto& signature = method.methodSignature ();
				if (receiver->metaObject ()->indexOfMethod (LC_N (signature)) == -1)
				{
					if (!destSlot)
					{
						qWarning () << Q_FUNC_INFO
								<< "not found meta method for"
								<< signature
								<< "in receiver object"
								<< receiver;
					}
					continue;
				}

				if (!QObject::connect (sender,
						LC_TOSIGNAL (signature),
						receiver,
						destSlot ? LC_TOSLOT (signature) : LC_TOSIGNAL (signature),
						Qt::UniqueConnection))
				{
					qWarning () << Q_FUNC_INFO
							<< "connect for"
							<< sender
							<< "->"
							<< receiver
							<< ":"
							<< signature
							<< "failed";
				}
			}
		}
#undef LC_N
	};

	void BaseHookInterconnector::AddPlugin (QObject *plugin)
	{
		Plugins_.push_back (plugin);

		ConnectHookSignals (this, plugin, true);
	}

	void BaseHookInterconnector::RegisterHookable (QObject *object)
	{
		ConnectHookSignals (object, this, false);
	}
}
}
