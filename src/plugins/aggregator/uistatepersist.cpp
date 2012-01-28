/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Alexander Konovalov
 * Copyright (C) 2006-2012  Georg Rudoy
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "uistatepersist.h"
#include <QTreeView>
#include <QString>
#include <QSettings>
#include <QDebug>
#include <QApplication>

namespace LeechCraft
{
namespace Aggregator
{
	void SaveColumnWidth (const QTreeView *tree, const QString& keyName)
	{
		// check that we can work with 'tree' argument
		if (!tree)
		{
			qWarning () << Q_FUNC_INFO << "tree == 0, not saving widths";
			return;
		}
		if (!tree->model ())
		{
			qWarning () << Q_FUNC_INFO << "tree->model () == 0, not saving widths";
			return;
		}
		// get width
		QList<QVariant> sizes;
		for (int i = 0, count = tree->model ()->columnCount (); i < count; ++i)
			sizes += tree->columnWidth (i);
		// save column width
		QSettings settings (QApplication::organizationName (), QApplication::applicationName () + "_Aggregator");
		settings.beginGroup ("tabs-width");
		settings.setValue (keyName, sizes);
		settings.endGroup ();
	}

	void LoadColumnWidth (QTreeView *tree, const QString& keyName)
	{
		// check that we can work with 'tree' argument
		if (!tree)
		{
			qWarning () << Q_FUNC_INFO << "tree == 0, not loading widths";
			return;
		}
		if (!tree->model ())
		{
			qWarning () << Q_FUNC_INFO << "tree->model () == 0, not loading widths";
			return;
		}
		// load column width
		QSettings settings (QApplication::organizationName (), QApplication::applicationName () + "_Aggregator");
		settings.beginGroup ("tabs-width");
		QList<QVariant> sizes = settings.value (keyName).toList ();
		settings.endGroup ();
		// column count check
		if (sizes.size () != tree->model ()->columnCount ())
		{
			qWarning () << Q_FUNC_INFO <<
				"Column count of tree (" << tree->model ()->columnCount () <<
				") != column count in settings (" << sizes.size () << ")";
			return;
		}
		// set width
		const int minColumnSize = 4;
		for (int i = 0; i < sizes.size (); i++)
		{
			// type check
			if (!sizes.at (i).canConvert<int> ())
			{
				qWarning() << Q_FUNC_INFO << "Can`t convert QVariant to int, "
					"(sizes[" << i << "]=" << sizes.at (i) << ")";
				return;
			}
			
			// min.size check
			int s = sizes.at (i).toInt ();
			if (s < minColumnSize)
			{
				qWarning() << Q_FUNC_INFO << "Size of column #" << i <<
					"(" << s << ") is too small (min." << minColumnSize << ")";
				continue;
			}
			
			tree->setColumnWidth (i, s);
		}
	}
}
}
