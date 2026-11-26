/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <util/compat/functional.h>

namespace LC::Azoth
{
	class IAccount;
	class IProtocol;

	class AccountsManager : public QObject
	{
		Q_OBJECT

		QList<IAccount*> Accounts_;
	public:
		using QObject::QObject;

		void OnEachAccount (const QObject& context, std::copyable_function<void (IAccount&)> action) const;

		void AddProtocol (IProtocol&);

		QList<IAccount*> GetAccounts () const;
	private:
		void RemoveAccount (IAccount&);
	private slots:
		void addAccount (QObject*);
		void removeAccount (QObject*);
	signals:
		void accountAdded (IAccount&);
		void accountRemoved (IAccount&);
	};
}
