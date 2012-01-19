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

#include "xmlimaccount.h"
#include <QDir>
#include <QStandardItemModel>
#include <QDomDocument>
#include "imimportpage.h"

namespace LeechCraft
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
		Q_FOREACH (const QString& path, C_.ProfilesPath_)
			if (!dir.cd (path))
			{
				qWarning () << Q_FUNC_INFO
						<< "cannot cd into"
						<< C_.ProfilesPath_.join ("/");
				return;
			}

		Q_FOREACH (const QString& entry,
				dir.entryList (QDir::NoDotAndDotDot | QDir::Dirs))
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
		Q_FOREACH (auto item, row)
			item->setEditable (false);

		item->appendRow (row);
	}
}
}
}
