/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGININTERFACE_TREEITEM_H
#define PLUGININTERFACE_TREEITEM_H
#include <QList>
#include <QVector>
#include <QMap>
#include <QVariant>
#include "piconfig.h"

namespace LeechCraft
{
	namespace Util
	{
		class TreeItem
		{
			QList<TreeItem*> Childs_;
			QMap<int, QVector<QVariant> > Data_;
			TreeItem *Parent_;
		public:
			PLUGININTERFACE_API TreeItem (const QList<QVariant>&, TreeItem *parent = 0);
			PLUGININTERFACE_API ~TreeItem ();

			PLUGININTERFACE_API void AppendChild (TreeItem*);
			PLUGININTERFACE_API void PrependChild (TreeItem*);
			PLUGININTERFACE_API void InsertChild (int, TreeItem*);
			PLUGININTERFACE_API int ChildPosition (TreeItem*);
			PLUGININTERFACE_API void RemoveChild (int);
			PLUGININTERFACE_API TreeItem* Child (int);
			PLUGININTERFACE_API int ChildCount () const;
			PLUGININTERFACE_API int ColumnCount (int = Qt::DisplayRole) const;
			PLUGININTERFACE_API QVariant Data (int, int = Qt::DisplayRole) const;
			PLUGININTERFACE_API void ModifyData (int, const QVariant&, int = Qt::DisplayRole);
			PLUGININTERFACE_API TreeItem* Parent ();
			PLUGININTERFACE_API int Row () const;
		};
	};
};

#endif

