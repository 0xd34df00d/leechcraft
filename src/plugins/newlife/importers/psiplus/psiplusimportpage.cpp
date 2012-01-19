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

#include "psiplusimportpage.h"
#include <QDir>
#include <QStandardItemModel>
#include <QDomDocument>
#include <QDateTime>
#include <QtDebug>
#include <util/util.h>
#include "importwizard.h"
#include "imhistimporterbase.h"

namespace LeechCraft
{
namespace NewLife
{
namespace Importers
{
	PsiPlusImportPage::PsiPlusImportPage (QWidget *parent)
	: Common::IMImportPage (parent)
	{
	}

	void PsiPlusImportPage::FindAccounts ()
	{
		QDir dir = QDir::home ();
		if (!dir.cd (".config") ||
				!dir.cd ("Psi+") ||
				!dir.cd ("profiles"))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot cd into ~/.config/Psi+/profiles";
			return;
		}

		Q_FOREACH (const QString& entry,
				dir.entryList (QDir::NoDotAndDotDot | QDir::Dirs))
			ScanProfile (dir.filePath (entry), entry);
	}

	void PsiPlusImportPage::ScanProfile (const QString& path, const QString& profileName)
	{
		QDir dir (path);
		if (!dir.exists ("accounts.xml"))
		{
			qWarning () << Q_FUNC_INFO
					<< "no accounts.xml in"
					<< path;
			return;
		}

		QFile file (dir.filePath ("accounts.xml"));
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
		AccountsModel_->appendRow (item);

		const auto& accs = doc.documentElement ().firstChildElement ("accounts");
		auto acc = accs.firstChildElement ();
		while (!acc.isNull ())
		{
			ScanAccount (item, acc);
			acc = acc.nextSiblingElement ();
		}

		Ui_.AccountsTree_->expand (item->index ());
	}

	void PsiPlusImportPage::ScanAccount (QStandardItem *item,
			const QDomElement& account)
	{
		auto tfd = [&account] (const QString& field)
			{ return account.firstChildElement (field).text (); };
		auto ifd = [&account, &tfd] (const QString& field)
			{ return tfd (field).toInt (); };
		auto bfd = [&account, &tfd] (const QString& field)
			{ return tfd (field) == "true"; };

		QStringList jids;

		const auto& rosterCache = account.firstChildElement ("roster-cache");
		auto rosterItem = rosterCache.firstChildElement ();
		while (!rosterItem.isNull ())
		{
			const QString& jid = rosterItem.firstChildElement ("jid").text ();
			if (!jid.isEmpty ())
				jids << jid;
			rosterItem = rosterItem.nextSiblingElement ();
		}

		QMap<QString, QVariant> accountData;
		accountData ["ParentProfile"] = item->text ();
		accountData ["Protocol"] = "xmpp";
		accountData ["Name"] = tfd ("name");
		accountData ["Enabled"] = bfd ("enabled");
		accountData ["Jid"] = tfd ("jid");
		accountData ["Port"] = ifd ("port");
		accountData ["Host"] = bfd ("use-host") ? tfd ("host") : QString ();
		accountData ["Contacts"] = jids;

		QList<QStandardItem*> row;
		row << new QStandardItem (accountData ["Name"].toString ());
		row << new QStandardItem (accountData ["Jid"].toString ());
		row << new QStandardItem ();
		row << new QStandardItem ();
		row.first ()->setData (accountData, Roles::AccountData);
		row [Column::ImportAcc]->setCheckState (Qt::Checked);
		row [Column::ImportAcc]->setCheckable (true);
		row [Column::ImportHist]->setCheckState (Qt::Checked);
		row [Column::ImportHist]->setCheckable (true);
		Q_FOREACH (auto item, row)
			item->setEditable (false);

		item->appendRow (row);
	}

	void PsiPlusImportPage::SendImportAcc (QStandardItem *accItem)
	{
		Entity e = Util::MakeEntity (QVariant (),
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/im-account-import");

		QVariantMap data = accItem->data (Roles::AccountData).toMap ();
		data.remove ("Contacts");
		e.Additional_ ["AccountData"] = data;

		emit gotEntity (e);
	}

	namespace
	{
		class HistImporter : public IMHistImporterBase
		{
			QString Profile_;
			QString AccName_;
			QString AccID_;
			QStringList JIDs_;
			QString CurrentJID_;

			QFile CurrentFile_;
		public:
			HistImporter (const QString& profile,
					const QString& accName, const QString& accId,
					const QStringList& jids, QObject *parent = 0)
			: IMHistImporterBase (parent)
			, Profile_ (profile)
			, AccName_ (accName)
			, AccID_ (accId)
			, JIDs_ (jids)
			{
			}
		protected:
			Entity GetEntityChunk ()
			{
				Entity e;
				if (JIDs_.isEmpty ())
					return e;

				const int lim = 1000;

				int counter = 0;
				QVariantList list;

				if (!OpenNextFile ())
					return e;

				while (CurrentFile_.isOpen ())
				{
					while (true)
					{
						if (CurrentFile_.atEnd ())
						{
							CurrentFile_.close ();
							break;
						}

						try
						{
							const auto& var = ParseLine (QString::fromUtf8 (CurrentFile_.readLine ()));
							if (!var.isNull ())
							{
								list << var;
								if (++counter > lim)
									break;
							}
						}
						catch (const std::exception& e)
						{
							qWarning () << Q_FUNC_INFO
									<< "error parsing line"
									<< e.what ();
						}
					}

					if (counter > lim)
						break;

					if (!CurrentFile_.isOpen () || CurrentFile_.atEnd ())
					{
						CurrentFile_.close ();

						if (!OpenNextFile ())
							break;
					}
				}

				if (counter)
				{
					e.Additional_ ["History"] = list;
					e.Mime_ = "x-leechcraft/im-history-import";
					e.Additional_ ["AccountName"] = AccName_;
					e.Additional_ ["AccountID"] = AccID_;
					e.Parameters_ = OnlyHandle | FromUserInitiated;
				}

				return e;
			}
		private:
			bool OpenNextFile ()
			{
				while (!CurrentFile_.isOpen ())
				{
					if (JIDs_.isEmpty ())
						return false;

					CurrentJID_ = JIDs_.takeFirst ();
					CurrentFile_.setFileName (GetFileName (CurrentJID_));
					if (!CurrentFile_.exists ())
						continue;

					if (!CurrentFile_.open (QIODevice::ReadOnly))
					{
						qWarning () << Q_FUNC_INFO
								<< "cannot open"
								<< CurrentFile_.fileName ()
								<< CurrentFile_.errorString ();
						continue;
					}

					if (JIDs_.isEmpty ())
						return false;
				}

				return true;
			}

			QString GetFileName (QString jid)
			{
				return QString ("%1/.local/share/Psi+/profiles/%2/history/%3.history")
						.arg (QDir::homePath ())
						.arg (Profile_)
						.arg (jid.replace ('@', "_at_"));
			}

			QVariant ParseLine (const QString& line)
			{
				QStringList list = line.split ('|', QString::SkipEmptyParts);
				if (list.size () < 5 || list [1] != "1")
					return QVariant ();

				QVariantMap result;
				result ["EntryID"] = CurrentJID_;
				result ["DateTime"] = QDateTime::fromString (list.at (0), Qt::ISODate);
				result ["Direction"] = list.at (2) == "to" ? "out" : "in";

				list.erase (list.begin (), list.begin () + 4);
				result ["Body"] = list.join ("|").trimmed ();
				result ["MessageType"] = "chat";
				return result;
			}
		};
	}

	void PsiPlusImportPage::SendImportHist (QStandardItem *accItem)
	{
		QVariantMap data = accItem->data (Roles::AccountData).toMap ();

		HistImporter *hi = new HistImporter (data ["ParentProfile"].toString (),
				data ["Name"].toString (),
				data ["Jid"].toString (),
				data ["Contacts"].toStringList ());
		connect (hi,
				SIGNAL (gotEntity (LeechCraft::Entity)),
				qobject_cast<ImportWizard*> (wizard ())->GetPlugin (),
				SIGNAL (gotEntity (LeechCraft::Entity)));
	}

	void PsiPlusImportPage::handleAccepted ()
	{
		for (int i = 0; i < AccountsModel_->rowCount (); ++i)
		{
			QStandardItem *profItem = AccountsModel_->item (i);
			for (int j = 0; j < profItem->rowCount (); ++j)
			{
				QStandardItem *accItem = profItem->child (j);
				if (profItem->child (j, Column::ImportAcc)->checkState () == Qt::Checked)
					SendImportAcc (accItem);
				if (profItem->child (j, Column::ImportHist)->checkState () == Qt::Checked)
					SendImportHist (accItem);
			}
		}
	}
}
}
}
