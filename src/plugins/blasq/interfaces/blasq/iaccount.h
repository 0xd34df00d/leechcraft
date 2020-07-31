/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include "collection.h"

class QAbstractItemModel;
class QString;

namespace LC
{
namespace Blasq
{
	class IService;

	class IAccount
	{
	public:
		virtual ~IAccount () {}

		virtual QObject* GetQObject () = 0;

		virtual IService* GetService () const = 0;

		virtual QString GetName () const = 0;

		virtual QByteArray GetID () const = 0;

		virtual QAbstractItemModel* GetCollectionsModel () const = 0;

		virtual void UpdateCollections () = 0;
	signals:
		virtual void doneUpdating () = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Blasq::IAccount, "org.LeechCraft.Blasq.IAccount/1.0")
