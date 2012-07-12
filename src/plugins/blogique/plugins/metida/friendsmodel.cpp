/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "friendsmodel.h"
#include <QtDebug>
#include <QMimeData>
#include "frienditemdelegate.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	FriendsModel::FriendsModel (QObject *parent)
	: QStandardItemModel (parent)
	{
	}

	Qt::ItemFlags FriendsModel::flags (const QModelIndex& index) const
	{
		Qt::ItemFlags defFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
		return index.parent ().isValid () ?
			Qt::ItemIsDragEnabled | defFlags:
			Qt::ItemIsDropEnabled | defFlags;
	}

	Qt::DropActions FriendsModel::supportedDropActions () const
	{
		return Qt::MoveAction;
	}

	QStringList FriendsModel::mimeTypes () const
	{
		return { "application/lj.friend" };
	}

	QMimeData* FriendsModel::mimeData (const QModelIndexList& indexes) const
	{
		QMimeData *mimeData = new QMimeData ();
		QByteArray encodedData;

		QDataStream stream (&encodedData, QIODevice::WriteOnly);

		Q_FOREACH (const QModelIndex& index, indexes)
			if (index.isValid ())
				stream << data (index).toString ()
						<< data (index, ItemColorRoles::BackgroundColor).toString ()
						<< data (index, ItemColorRoles::ForegroundColor).toString ()
						<< index.parent ().data (ItemGroupRoles::GroupId).toInt ();

		mimeData->setData ("application/lj.friend", encodedData);
		return mimeData;
	}

	bool FriendsModel::dropMimeData (const QMimeData *mime,
			Qt::DropAction action, int row, int column, const QModelIndex& parent)
	{
		if (action == Qt::IgnoreAction)
			return true;

		int newGrp = parent.isValid () ?
			parent.data (ItemGroupRoles::GroupId).toInt () :
			-1;

		QDataStream stream (mime->data ("application/lj.friend"));
		QString name, bgColor, fgColor;
		int id = -1;
		stream >> name
				>> bgColor
				>> fgColor
				>> id;

		if (newGrp == id)
			return false;

		emit userGroupChanged (name, bgColor, fgColor, newGrp);
		return true;
	}

}
}
}

