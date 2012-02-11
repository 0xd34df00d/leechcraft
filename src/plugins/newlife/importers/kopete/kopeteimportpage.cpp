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

#include "kopeteimportpage.h"
#include <QDir>
#include <QStandardItemModel>
#include "kopeteimportthread.h"

namespace LeechCraft
{
namespace NewLife
{
namespace Importers
{
	KopeteImportPage::KopeteImportPage (QWidget *parent)
	: Common::IMImportPage (parent)
	{
	}

	void KopeteImportPage::FindAccounts ()
	{
		QDir dir = QDir::home ();
		QStringList path;
		path << ".kde4" << "share" << "apps" << "kopete" << "logs";
		Q_FOREACH (const QString& str, path)
			if (!dir.cd (str))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd into"
						<< str;
				return;
			}

		QStringList knownProtos;
		knownProtos << "JabberProtocol" << "ICQProtocol";
		Q_FOREACH (const QString& proto, knownProtos)
			if (dir.exists (proto))
				ScanProto (dir.filePath (proto),
						proto.left (proto.size () - QString ("Protocol").size ()));

		Ui_.AccountsTree_->expandAll ();
	}

	void KopeteImportPage::SendImportAcc (QStandardItem*)
	{
	}

	void KopeteImportPage::SendImportHist (QStandardItem *accItem)
	{
		const QVariantMap& data = accItem->data (Roles::AccountData).toMap ();
		const QString& path = data ["LogsPath"].toString ();

		QDir dir (path);
		QStringList paths;
		Q_FOREACH (const QString& file, dir.entryList (QDir::Files))
			paths << dir.absoluteFilePath (file);

		KopeteImportThread *thread = new KopeteImportThread (data ["Protocol"].toString (), paths);
		connect (thread,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				S_Plugin_,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				Qt::QueuedConnection);
		thread->start (QThread::LowestPriority);
	}

	void KopeteImportPage::ScanProto (const QString& path, const QString& proto)
	{
		QStandardItem *protoItem = new QStandardItem (proto);
		AccountsModel_->appendRow (protoItem);

		QMap<QString, QString> kopeteProto2LC;
		kopeteProto2LC ["Jabber"] = "xmpp";
		kopeteProto2LC ["ICQ"] = "oscar";
		kopeteProto2LC ["IRC"] = "irc";

		if (!kopeteProto2LC.contains (proto))
			return;

		const QDir dir (path);
		Q_FOREACH (const QString& accDirName,
				dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
		{
			QDir accDir = dir;
			if (!accDir.cd (accDirName))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd to"
						<< accDirName
						<< path;
				continue;
			}

			QString accId = accDirName;
			accId.replace ('-', '.');

			QVariantMap accountData;
			accountData ["Jid"] = accId;
			accountData ["Protocol"] = kopeteProto2LC.value (proto);
			accountData ["LogsPath"] = accDir.absolutePath ();

			QList<QStandardItem*> row;
			row << new QStandardItem (accId);
			row << new QStandardItem (accId);
			row << new QStandardItem ();
			row << new QStandardItem ();
			row.first ()->setData (accountData, IMImportPage::Roles::AccountData);
			row [IMImportPage::Column::ImportAcc]->setCheckState (Qt::Checked);
			row [IMImportPage::Column::ImportAcc]->setCheckable (true);
			row [IMImportPage::Column::ImportAcc]->setEnabled (false);
			row [IMImportPage::Column::ImportHist]->setCheckState (Qt::Checked);
			row [IMImportPage::Column::ImportHist]->setCheckable (true);
			Q_FOREACH (auto item, row)
				item->setEditable (false);

			protoItem->appendRow (row);
		}
	}
}
}
}
