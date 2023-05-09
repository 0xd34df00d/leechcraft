/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>

class QModelIndex;
class QTreeView;

class Q_DECL_EXPORT ISummaryRepresentation
{
public:
	virtual ~ISummaryRepresentation () = default;

	virtual QModelIndex MapToSource (const QModelIndex&) const = 0;
};

Q_DECLARE_INTERFACE (ISummaryRepresentation, "org.Deviant.LeechCraft.ISummaryRepresentation/1.0")
