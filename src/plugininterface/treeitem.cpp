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

#include "treeitem.h"
#include <QStringList>
#include <QDebug>

using namespace LeechCraft::Util;

TreeItem::TreeItem (const QList<QVariant>& data, TreeItem *parent)
: Parent_ (parent)
{
	Data_ [Qt::DisplayRole] = data.toVector ();
}

TreeItem::~TreeItem ()
{
	qDeleteAll (Children_);
}

void TreeItem::AppendChild (TreeItem *child)
{
	Children_.append (child);
}

void TreeItem::PrependChild (TreeItem *child)
{
	Children_.prepend (child);
}

void TreeItem::InsertChild (int index, TreeItem *child)
{
	Children_.insert (index, child);
}

int TreeItem::ChildPosition (const TreeItem *child) const
{
	return Children_.indexOf (const_cast<TreeItem*> (child));
}

void TreeItem::RemoveChild (int child)
{
	delete Children_.takeAt (child);
}

TreeItem* TreeItem::Child (int row) const
{
	return Children_.value (row);
}

int TreeItem::ChildCount () const
{
	return Children_.count ();
}

int TreeItem::ColumnCount (int role) const
{
	return Data_ [role].count ();
}

QVariant TreeItem::Data (int column, int role) const
{
	return Data_ [role].value (column);
}

void TreeItem::ModifyData (int column, const QVariant& data, int role)
{
	if (Data_ [role].size () <= column)
		Data_ [role].resize (column + 1);
	Data_ [role] [column] = data;
}

const TreeItem* TreeItem::Parent () const
{
	return Parent_;
}

TreeItem* TreeItem::Parent ()
{
	return Parent_;
}

int TreeItem::Row () const
{
	if (Parent_)
		return Parent_->Children_.indexOf (const_cast<TreeItem*> (this));
	return 0;
}

QDebug operator<< (QDebug dbg, const LeechCraft::Util::TreeItem& item)
{
	dbg.nospace () << "{ TreeItem| Parent:"
		<< item.Parent ();
	dbg.nospace () << "; Children:"
		<< item.ChildCount ();
	dbg.nospace () << "; Data[0]:"
		<< (item.ColumnCount () ? item.Data (0) : QString ("<no columns>"))
		<< "}";
	return dbg.space ();
}

QDebug operator<< (QDebug dbg, const LeechCraft::Util::TreeItem* const pitem)
{
	dbg.nospace () << "TreeItem @ " 
		<< static_cast<const void* const> (pitem)
		<< ";";
	if (pitem)
		dbg << *pitem;
	return dbg.space ();
}

