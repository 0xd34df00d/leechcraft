/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AGGREGATOR_UISTATEPERSIST_H
#define PLUGINS_AGGREGATOR_UISTATEPERSIST_H
#include <QTreeView>
#include <QString>

namespace LC
{
namespace Aggregator
{
	/** Save column width of tree to aggregator`s section of settings */
	void SaveColumnWidth (const QTreeView *tree, const QString& keyName);

	/** Try to load column width of tree from aggregator`s section of settings */
	void LoadColumnWidth (QTreeView *tree, const QString& keyName);
}
}

#endif
