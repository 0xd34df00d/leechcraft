/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "xmlimaccount.h"
#include <QDir>
#include <QStandardItemModel>
#include <QDomDocument>
#include "imimportpage.h"

namespace LC
{
namespace NewLife
{
namespace Common
{
	XMLIMAccount::XMLIMAccount (const XMLIMAccount::ConfigAdapter& adapter)
	: C_ (adapter)
	{
	}

	void XMLIMAccount::FindAccounts ()
	{
		QDir dir = QDir::home ();
		for (auto path : C_.ProfilesPath_)
		{
			if (!dir.cd (path))
			{
				const auto& list = dir.entryList (QDir::AllDirs | QDir::Hidden | QDir::NoDotAndDotDot);
				for (const auto& candidate : list)
					if (!QString::compare (candidate, path, Qt::CaseInsensitive))
					{
						path = candidate;
						break;
					}
			}
			else
				continue;

			if (!dir.cd (path))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot cd into"
						<< path
						<< C_.ProfilesPath_.join ("/");
				return;
			}
		}

		for (const auto& entry : dir.entryList (QDir::NoDotAndDotDot | QDir::Dirs))
			ScanProfile (dir.filePath (entry), entry);
	}

	void XMLIMAccount::ScanProfile (const QString& path, const QString& profileName)
	{
		QDir dir (path);
		if (!dir.exists (C_.AccountsFileName_))
		{
			qWarning () << Q_FUNC_INFO
					<< "no accounts.xml in"
					<< path;
			return;
		}

		QFile file (dir.filePath (C_.AccountsFileName_));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		QDomDocument doc;

		QString error;
		int line = 0, col = 0;
		if (!doc.setContent (&file, &error, &line, &col))
		{
			qWarning () << Q_FUNC_INFO
					<< "failed to parse"
					<< file.fileName ()
					<< error
					<< line
					<< col;
			return;
		}

		QStandardItem *item = new QStandardItem (profileName);
		item->setEditable (false);
		C_.Model_->appendRow (item);

		const auto& accs = doc.documentElement ().firstChildElement ("accounts");
		auto acc = accs.firstChildElement ();
		while (!acc.isNull ())
		{
			ScanAccount (item, acc);
			acc = acc.nextSiblingElement ();
		}
	}

	void XMLIMAccount::ScanAccount (QStandardItem *item, const QDomElement& account)
	{
		QVariantMap accountData;
		accountData ["ParentProfile"] = item->text ();
		accountData ["Protocol"] = C_.Protocol_ (account);
		accountData ["Name"] = C_.Name_ (account);
		accountData ["Enabled"] = C_.IsEnabled_ (account);
		accountData ["Jid"] = C_.JID_ (account);
		C_.Additional_ (account, accountData);

		QList<QStandardItem*> row;
		row << new QStandardItem (accountData ["Name"].toString ());
		row << new QStandardItem (accountData ["Jid"].toString ());
		row << new QStandardItem ();
		row << new QStandardItem ();
		row.first ()->setData (accountData, IMImportPage::Roles::AccountData);
		row [IMImportPage::Column::ImportAcc]->setCheckState (Qt::Checked);
		row [IMImportPage::Column::ImportAcc]->setCheckable (true);
		row [IMImportPage::Column::ImportHist]->setCheckState (Qt::Checked);
		row [IMImportPage::Column::ImportHist]->setCheckable (true);
		for (auto item : row)
			item->setEditable (false);

		item->appendRow (row);
	}
}
}
}
