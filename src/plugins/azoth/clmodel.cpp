/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "clmodel.h"
#include <QMimeData>
#include "interfaces/iclentry.h"
#include "interfaces/iaccount.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	const QString CLEntryFormat = "x-leechcraft/azoth-cl-entry";
	CLModel::CLModel (QObject *parent)
	: QStandardItemModel (parent)
	{
	}

	QStringList CLModel::mimeTypes () const
	{
		return QStringList (CLEntryFormat);
	}

	QMimeData* CLModel::mimeData (const QModelIndexList& indexes) const
	{
		QMimeData *result = new QMimeData;

		QByteArray encoded;
		QDataStream stream (&encoded, QIODevice::WriteOnly);

		Q_FOREACH (const QModelIndex& index, indexes)
		{
			if (index.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
				continue;

			QObject *entryObj = index
					.data (Core::CLREntryObject).value<QObject*> ();
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry ||
					entry->GetEntryType () != ICLEntry::ETChat)
				continue;

			const QString& thisGroup = index.parent ()
					.data (Core::CLREntryCategory).toString ();

			stream << entry->GetEntryID () << thisGroup;
		}

		result->setData (CLEntryFormat, encoded);

		return result;
	}

	bool CLModel::dropMimeData (const QMimeData *mime,
			Qt::DropAction action, int row, int column, const QModelIndex& parent)
	{
		if (action == Qt::IgnoreAction)
			return true;

		if (TryDropContact (mime, row, parent) ||
				TryDropContact (mime, parent.row (), parent.parent ()))
			return true;

		return false;
	}

	bool CLModel::TryDropContact (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		if (!mime->hasFormat (CLEntryFormat))
			return false;

		if (parent.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETAccount)
			return false;

		QObject *accObj = parent.data (Core::CLRAccountObject).value<QObject*> ();
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		if (!acc)
			return false;

		const QString& newGrp = parent.child (row, 0)
				.data (Core::CLREntryCategory).toString ();

		QDataStream stream (mime->data (CLEntryFormat));
		while (!stream.atEnd ())
		{
			QString id;
			QString oldGroup;
			stream >> id >> oldGroup;

			if (oldGroup == newGrp)
				continue;

			QObject *entryObj = Core::Instance ().GetEntry (id);
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
			if (!entry)
				continue;

			QStringList groups = entry->Groups ();
			groups.removeAll (oldGroup);
			groups << newGrp;

			entry->SetGroups (groups);
		}

		return true;
	}
}
}
