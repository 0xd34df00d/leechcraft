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

#include "clmodel.h"
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QMessageBox>
#include <util/defaulthookproxy.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "core.h"
#include "transferjobmanager.h"

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
		return QStringList (CLEntryFormat) << "text/uri-list" << "text/plain";
	}

	QMimeData* CLModel::mimeData (const QModelIndexList& indexes) const
	{
		QMimeData *result = new QMimeData;

		QByteArray encoded;
		QDataStream stream (&encoded, QIODevice::WriteOnly);

		QStringList names;

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

			names << entry->GetEntryName ();
		}

		result->setData (CLEntryFormat, encoded);
		result->setText (names.join ("; "));

		return result;
	}

	bool CLModel::dropMimeData (const QMimeData *mime,
			Qt::DropAction action, int row, int column, const QModelIndex& parent)
	{
		qDebug () << "drop" << mime->formats () << action;
		if (action == Qt::IgnoreAction)
			return true;

		if (PerformHooks (mime, row, parent))
			return true;

		if (TryDropContact (mime, row, parent) ||
				TryDropContact (mime, parent.row (), parent.parent ()))
			return true;

		if (TryDropFile (mime, parent))
			return true;

		return false;
	}

	Qt::DropActions CLModel::supportedDropActions () const
	{
		return static_cast<Qt::DropActions> (Qt::CopyAction | Qt::MoveAction | Qt::LinkAction);
	}

	bool CLModel::PerformHooks (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		if (CheckHookDnDEntry2Entry (mime, row, parent))
			return true;

		return false;
	}

	bool CLModel::CheckHookDnDEntry2Entry (const QMimeData *mime, int row, const QModelIndex& parent)
	{
		if (row != -1 ||
				!mime->hasFormat (CLEntryFormat) ||
				parent.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return false;

		QDataStream stream (mime->data (CLEntryFormat));
		QString sid;
		stream >> sid;

		QObject *source = Core::Instance ().GetEntry (sid);
		if (!source)
			return false;

		QObject *target = parent.data (Core::CLREntryObject).value<QObject*> ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookDnDEntry2Entry (proxy, source, target);
		return proxy->IsCancelled ();
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

	bool CLModel::TryDropFile (const QMimeData* mime, const QModelIndex& parent)
	{
		if (parent.data (Core::CLREntryType).value<Core::CLEntryType> () != Core::CLETContact)
			return false;

		QObject *entryObj = parent.data (Core::CLREntryObject).value<QObject*> ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		if (entry->Variants ().isEmpty ())
			return false;

		IAccount *acc = qobject_cast<IAccount*> (entry->GetParentAccount ());
		ITransferManager *mgr = qobject_cast<ITransferManager*> (acc->GetTransferManager ());
		if (!mgr)
			return false;

		const QList<QUrl>& urls = mime->urls ();
		if (urls.isEmpty ())
			return false;

		QString text;
		if (urls.size () > 2)
			text = tr ("Are you sure you want to send %n files to %1?", 0, urls.size ())
					.arg (entry->GetEntryName ());
		else
		{
			QStringList list;
			Q_FOREACH (const QUrl& url, urls)
				list << QFileInfo (url.path ()).fileName ();
			text = tr ("Are you sure you want to send %1 to %2?")
					.arg ("<em>" + list.join (", ") + "</em>")
					.arg (entry->GetEntryName ());
		}
		if (QMessageBox::question (0,
					"LeechCraft",
					text,
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return false;

		Q_FOREACH (const QUrl& url, urls)
		{
			const QString& path = url.toLocalFile ();

			if (!QFileInfo (path).exists ())
				continue;

			QObject *job = mgr->SendFile (entry->GetEntryID (),
					entry->Variants ().first (), path);
			Core::Instance ().GetTransferJobManager()->HandleJob (job);
		}

		return true;
	}
}
}
