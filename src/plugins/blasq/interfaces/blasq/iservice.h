/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QList>

class QString;
class QIcon;
class QWidget;

namespace LC
{
namespace Blasq
{
	class IAccount;

	class IService
	{
	public:
		virtual ~IService () {}

		virtual QObject* GetQObject () = 0;

		virtual QString GetServiceName () const = 0;

		virtual QIcon GetServiceIcon () const = 0;

		virtual QList<IAccount*> GetRegisteredAccounts () const = 0;

		virtual QList<QWidget*> GetAccountRegistrationWidgets () const = 0;

		virtual void RegisterAccount (const QString& name, const QList<QWidget*>& widgets) = 0;

		virtual void RemoveAccount (IAccount*) = 0;
	protected:
		virtual void accountAdded (QObject*) = 0;

		virtual void accountRemoved (QObject*) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blasq::IService, "org.LeechCraft.Blasq.IService/1.0")
