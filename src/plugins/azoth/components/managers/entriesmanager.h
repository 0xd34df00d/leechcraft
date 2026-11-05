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

		QHash<GlobalStrongestId, ICLEntry*> Id2Entry_;
	public:
		using QObject::QObject;

		void AddAccount (IAccount&);

		template<typename T = ICLEntry>
		T& GetEntry (const GlobalStrongestId&) const;

		template<typename T = ICLEntry>
		T* GetEntryOrNull (const GlobalStrongestId&) const;

		template<typename T = ICLEntry>
		T* GetEntryOrNull (const std::optional<GlobalStrongestId>& id) const
		{
			return id ? GetEntryOrNull<T> (*id) : nullptr;
		}
	private:
		void AddCLEntries (const QList<ICLEntry*>&, IAccount&);
		void RemoveCLEntries (const QList<ICLEntry*>&, IAccount&);
	signals:
		void added (const QList<ICLEntry*>&, IAccount&);
		void aboutToBeRemoved (const QList<ICLEntry*>&, IAccount&);

		void strongestIdChanged (const GlobalConventionalId& oldId, const GlobalConventionalId& newId, const ICLEntry& entry);
	};
}
