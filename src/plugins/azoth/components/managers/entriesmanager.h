/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/azoth/azothcommon.h"

namespace LC::Azoth
{
	class IAccount;
	class ICLEntry;

	class EntriesManager : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;

		void AddAccount (IAccount&);
	signals:
		void conventionalIdChanged (const GlobalConventionalId& oldId, const GlobalConventionalId& newId, const ICLEntry& entry);
	};
}
