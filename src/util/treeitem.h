/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#ifndef UTIL_TREEITEM_H
#define UTIL_TREEITEM_H
#include <QList>
#include <QVector>
#include <QMap>
#include <QVariant>
#include "utilconfig.h"

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem
		{
			QList<TreeItem*> Children_;
			QMap<int, QVector<QVariant>> Data_;
			TreeItem *Parent_;
		public:
			UTIL_API TreeItem (const QList<QVariant>&, TreeItem *parent = 0);
			UTIL_API ~TreeItem ();

			UTIL_API void AppendChild (TreeItem*);
			UTIL_API void PrependChild (TreeItem*);
			UTIL_API void InsertChild (int, TreeItem*);
			UTIL_API int ChildPosition (const TreeItem*) const;
			UTIL_API void RemoveChild (int);
			UTIL_API TreeItem* Child (int) const;
			UTIL_API int ChildCount () const;
			UTIL_API int ColumnCount (int = Qt::DisplayRole) const;
			UTIL_API QVariant Data (int, int = Qt::DisplayRole) const;
			UTIL_API void ModifyData (int, const QVariant&, int = Qt::DisplayRole);
			UTIL_API const TreeItem* Parent () const;
			UTIL_API TreeItem* Parent ();
			UTIL_API int Row () const;
		};
	};
};

UTIL_API QDebug operator<< (QDebug, const LeechCraft::Util::TreeItem&);
UTIL_API QDebug operator<< (QDebug, const LeechCraft::Util::TreeItem* const);

#endif

