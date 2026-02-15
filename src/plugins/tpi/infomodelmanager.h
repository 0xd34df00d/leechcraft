/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/ijobholder.h>

class QAbstractItemModel;
class QConcatenateTablesProxyModel;
class QIdentityProxyModel;
class QSortFilterProxyModel;

namespace LC
{
namespace TPI
{
	class InfoModelManager : public QObject
	{
		QConcatenateTablesProxyModel& Concat_;
		QSortFilterProxyModel& Filter_;
		QIdentityProxyModel& Structurize_;

		std::vector<IJobHolderRepresentationHandler_ptr> Handlers_;
	public:
		explicit InfoModelManager (QObject* = nullptr);

		QAbstractItemModel* GetModel () const;

		void SecondInit ();
	};
}
}
