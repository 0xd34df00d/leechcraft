/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QtPlugin>

class QObject;

namespace Laretz
{
	class Item;
	class Operation;
}

class ISyncProxy
{
public:
	virtual ~ISyncProxy () {}

	virtual QObject* GetQObject () = 0;

	virtual QList<Laretz::Operation> GetAllOps () const = 0;

	virtual QList<Laretz::Operation> GetNewOps () const = 0;

	virtual void Merge (QList<Laretz::Operation>& ours, const QList<Laretz::Operation>& theirs) = 0;
protected:
	virtual void gotNewOps (const QList<Laretz::Operation>&) = 0;
};

class Q_DECL_EXPORT ISyncable
{
public:
	virtual ~ISyncable () {}

	virtual ISyncProxy* GetSyncProxy () = 0;
};

Q_DECLARE_INTERFACE (ISyncProxy, "org.Deviant.LeechCraft.ISyncProxy/1.0")
Q_DECLARE_INTERFACE (ISyncable, "org.Deviant.LeechCraft.ISyncable/1.0")
