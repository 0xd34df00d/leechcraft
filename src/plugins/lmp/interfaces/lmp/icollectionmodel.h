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

class QModelIndex;
class QUrl;

namespace LC
{
namespace LMP
{
	class ICollectionModel
	{
	protected:
		virtual ~ICollectionModel () {}
	public:
		virtual QList<QUrl> ToSourceUrls (const QList<QModelIndex>& indexes) const = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::LMP::ICollectionModel, "org.LeechCraft.LMP.ICollectionModel/1.0")
