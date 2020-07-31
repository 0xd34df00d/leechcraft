/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/isyncable.h>

namespace LC
{
namespace Otlozhu
{
	class SyncProxy : public QObject
					, public ISyncProxy
	{
		Q_OBJECT
		Q_INTERFACES (ISyncProxy)
	public:
		SyncProxy (QObject* = 0);

		QObject* GetQObject ();
		QList<Laretz::Operation> GetAllOps () const;
		QList<Laretz::Operation> GetNewOps () const;
		void Merge (QList<Laretz::Operation>& ours, const QList<Laretz::Operation>& theirs);
	signals:
		void gotNewOps (const QList<Laretz::Operation>&);
	};
}
}
