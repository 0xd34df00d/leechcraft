/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "kopeteimportpage.h"
#include <QDir>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/sll/prelude.h>
#include "kopeteimportthread.h"

namespace LC
{
namespace NewLife
{
namespace Importers
{
	KopeteImportPage::KopeteImportPage (const ICoreProxy_ptr& proxy, QWidget *parent)
	: Common::IMImportPage { proxy, parent }
	{
	}

	void KopeteImportPage::FindAccounts ()
	{
		QDir dir = QDir::home ();
		for (const auto& str : { ".kde4", "share", "apps", "kopete", "logs" })
			if (!dir.cd (str))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cd into"
						<< str;
				return;
			}

		for (QString proto : { "JabberProtocol", "ICQProtocol" })
			if (dir.exists (proto))
			{
				proto.chop (QByteArray { "Protocol" }.size ());
				ScanProto (dir.filePath (proto), proto);
			}

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
		const auto& paths = Util::Map (dir.entryList (QDir::Files),
				[&dir] (const QString& file) { return dir.absoluteFilePath (file); });

		const auto thread = new KopeteImportThread (Proxy_, data ["Protocol"].toString (), paths);
		thread->start (QThread::LowestPriority);
	}

	void KopeteImportPage::ScanProto (const QString& path, const QString& proto)
	{
		QStandardItem *protoItem = new QStandardItem (proto);
		AccountsModel_->appendRow (protoItem);

		static const QMap<QString, QString> kopeteProto2LC
		{
			{ "Jabber", "xmpp" },
			{ "ICQ", "oscar" },
			{ "IRC", "irc" }
		};

		if (!kopeteProto2LC.contains (proto))
			return;

		const QDir dir (path);
		for (const auto& accDirName : dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
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

			const QVariantMap accountData
			{
				{ "Jid", accId },
				{ "Protocol", kopeteProto2LC.value (proto) },
				{ "LogsPath", accDir.absolutePath () }
			};

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
			for (auto item : row)
				item->setEditable (false);

			protoItem->appendRow (row);
		}
	}
}
}
}
