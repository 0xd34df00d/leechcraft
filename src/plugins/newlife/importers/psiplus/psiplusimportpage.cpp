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
#include <QDomElement>
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
		auto tfd = [] (const QDomElement& account, const QString& field)
			{ return account.firstChildElement (field).text (); };

		auto adapter = Common::XMLIMAccount::ConfigAdapter
		{
			AccountsModel_,
			QStringList (".config") << "Psi+" << "profiles",
			"accounts.xml",
			[] (const QDomElement&) { return "xmpp"; },
			[tfd] (const QDomElement& acc) { return tfd (acc, "name"); },
			[tfd] (const QDomElement& acc) { return tfd (acc, "enabled") == "true"; },
			[tfd] (const QDomElement& acc) { return tfd (acc, "jid"); },
			[tfd] (const QDomElement& acc, QVariantMap& accountData)
			{
				accountData ["Port"] = tfd (acc, "port").toInt ();
				accountData ["Host"] = tfd (acc, "use-host") == "true" ?
						tfd (acc, "host") :
						QString ();

				QStringList jids;

				auto rosterItem = acc.firstChildElement ("roster-cache").firstChildElement ();
				while (!rosterItem.isNull ())
				{
					const QString& jid = rosterItem.firstChildElement ("jid").text ();
					if (!jid.isEmpty ())
						jids << jid;
					rosterItem = rosterItem.nextSiblingElement ();
				}
				accountData ["Contacts"] = jids;
			}
		};
		XIA_.reset (new Common::XMLIMAccount (adapter));
	}

	void PsiPlusImportPage::FindAccounts ()
	{
		XIA_->FindAccounts ();
		Ui_.AccountsTree_->expandAll ();
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
}
}
}
