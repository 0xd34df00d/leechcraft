/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>

class QAction;
class QTreeView;

namespace LC
{
namespace NetStoreManager
{
	class FilesTreeModel : public QStandardItemModel
	{
	public:
		using QStandardItemModel::QStandardItemModel;

		Qt::DropActions supportedDropActions () const override;
		QStringList mimeTypes () const override;
		QMimeData* mimeData (const QModelIndexList& indexes) const override;
	};
}
}

