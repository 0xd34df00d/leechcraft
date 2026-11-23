/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/azothcommon.h>
#include "../../azothutilconfig.h"

namespace LC::Azoth
{
	class IAccount;
}

namespace LC::Azoth::Emitters
{
	class AZOTH_UTIL_API EntriesManager : public QObject
	{
		Q_OBJECT
	public:
		using QObject::QObject;
	signals:
		void added (const QList<ICLEntry*>&, IAccount&);
		void aboutToBeRemoved (const QList<ICLEntry*>&, IAccount&);
		void removed (const QList<ICLEntry*>&, IAccount&);

		void strongestIdChanged (const GlobalConventionalId& oldId, const GlobalConventionalId& newId, const ICLEntry& entry);
		void stableIdShadowed (quint64 oldId);
	};
}
