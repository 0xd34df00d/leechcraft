/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/ifinder.h>
#include "searchhandler.h"

namespace LC
{
namespace Util
{
	class MergeModel;
};

namespace SeekThru
{
	class FindProxy : public QObject
					, public IFindProxy
	{
		Q_OBJECT
		Q_INTERFACES (IFindProxy)

		Request R_;
		std::shared_ptr<LC::Util::MergeModel> MergeModel_;
		QList<SearchHandler_ptr> Handlers_;
	public:
		explicit FindProxy (Request);
		~FindProxy () override;

		QAbstractItemModel* GetModel () override;
		QByteArray GetUniqueSearchID () const override;
		QStringList GetCategories () const override;

		void SetHandlers (const QList<SearchHandler_ptr>&);
	};
}
}
