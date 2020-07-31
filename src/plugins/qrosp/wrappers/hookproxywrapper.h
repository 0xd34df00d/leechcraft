/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_QROSP_WRAPPERS_HOOKPROXYWRAPPER_H
#define PLUGINS_QROSP_WRAPPERS_HOOKPROXYWRAPPER_H
#include <memory>
#include <QObject>
#include <QVariant>

namespace LC
{
class IHookProxy;
typedef std::shared_ptr<IHookProxy> IHookProxy_ptr;

namespace Qrosp
{
	class HookProxyWrapper : public QObject
	{
		Q_OBJECT
		Q_PROPERTY (QVariant ReturnValue READ GetReturnValue WRITE SetReturnValue)

		IHookProxy_ptr Proxy_;
	public:
		HookProxyWrapper (IHookProxy_ptr);
	public slots:
		void CancelDefault ();
		const QVariant& GetReturnValue () const;
		void SetReturnValue (const QVariant&);
		void SetValue (const QByteArray&, const QVariant&);
	};
}
}

#endif
