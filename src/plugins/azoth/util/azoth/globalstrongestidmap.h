/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSet>
#include <interfaces/azoth/azothcommon.h>
#include "azothutilconfig.h"

namespace LC::Azoth
{
	class IAccount;
	class ICLEntry;

	class AZOTH_UTIL_API GlobalStrongestIdMap : public QObject
	{
		QSet<IAccount*> KnownAccounts_;
		QHash<GlobalStrongestId, ICLEntry*> Entries_;
	public:
		using QObject::QObject;

		void AddEntry (ICLEntry& entry);
		ICLEntry* GetEntry (const GlobalStrongestId& id) const;
	private:
		void RegisterAccount (IAccount*);
	};
}
