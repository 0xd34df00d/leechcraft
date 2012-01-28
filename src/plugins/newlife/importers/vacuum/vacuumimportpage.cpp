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

#include "vacuumimportpage.h"
#include <QStandardItemModel>
#include <QDomDocument>
#include <QDateTime>
#include <QDir>
#include <util/util.h>

namespace LeechCraft
{
namespace NewLife
{
namespace Importers
{
	VacuumImportPage::VacuumImportPage (QWidget *parent)
	: Common::IMImportPage (parent)
	{
		auto tfd = [] (const QDomElement& account, const QString& field)
			{ return account.firstChildElement (field).text (); };

		auto adapter = Common::XMLIMAccount::ConfigAdapter
		{
			AccountsModel_,
			QStringList (".vacuum") << "profiles",
			"options.xml",
			[] (const QDomElement&) { return "xmpp"; },
			[tfd] (const QDomElement& acc) { return tfd (acc, "name"); },
			[tfd] (const QDomElement& acc) { return tfd (acc, "active") == "true"; },
			[tfd] (const QDomElement& acc)
			{
				const auto& sjid = tfd (acc, "streamJid");
				const int pos = sjid.indexOf ('/');
				return pos < 0 ? sjid : sjid.left (pos);
			},
			[tfd] (const QDomElement& acc, QVariantMap& accountData)
			{
				const QDomElement& conn = acc.firstChildElement ("connection");

				const int port = tfd (conn, "port").toInt ();
				accountData ["Port"] = port ? port : 5222;

				const QString& host = tfd (conn, "host");
				if (!host.isEmpty ())
					accountData ["Host"] = host;
			}
		};
		XIA_.reset (new Common::XMLIMAccount (adapter));
	}

	void VacuumImportPage::FindAccounts ()
	{
		XIA_->FindAccounts ();
		Ui_.AccountsTree_->expandAll ();
	}

	void VacuumImportPage::SendImportAcc (QStandardItem *accItem)
	{
		Entity e = Util::MakeEntity (QVariant (),
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/im-account-import");
		e.Additional_ ["AccountData"] = accItem->data (Roles::AccountData);
		emit gotEntity (e);
	}

	namespace
	{
		void ParseFile (const QString& path, QVariantList& result, const QVariantMap& data)
		{
			QFile file (path);
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
						<< "XML error:"
						<< error
						<< line
						<< col;
				return;
			}

			const auto& chatElem = doc.documentElement ();
			auto currentDateTime = QDateTime::fromString (chatElem.attribute ("start"), Qt::ISODate);
			if (currentDateTime.isNull ())
			{
				qWarning () << Q_FUNC_INFO
						<< "null initial datetime";
				return;
			}

			const QString& fullJID = chatElem.attribute ("with");
			const int pos = fullJID.indexOf ('/');
			const QString& bareJID = pos > 0 ?
					fullJID.left (pos) :
					fullJID;
			const QString& variant = pos > 0 ?
					fullJID.mid (pos + 1) :
					QString ();

			QVariantMap initial;
			initial ["Variant"] = variant;
			initial ["EntryID"] = bareJID;
			initial ["MessageType"] = "chat";

			QDomElement child = chatElem.firstChildElement ();
			while (!child.isNull ())
			{
				currentDateTime = currentDateTime.addSecs (child.attribute ("secs").toInt ());

				QVariantMap map = initial;
				map ["Direction"] = child.tagName () == "from" ? "in" : "out";
				map ["Body"] = child.firstChildElement ("body").text ();
				map ["DateTime"] = currentDateTime;
				result << map;

				child = child.nextSiblingElement ();
			}
		}

		QVariantList HandleJID (QDir dir, const QString& encJid, const QVariantMap& data)
		{
			if (!dir.cd (encJid))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to CD into"
						<< encJid;
				return QVariantList ();
			}

			QVariantList result;
			Q_FOREACH (const QString& variant,
					dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
			{
				QDir varDir (dir);
				if (!varDir.cd (variant))
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cd into"
							<< encJid
							<< variant;
					continue;
				}

				Q_FOREACH (const QString& logFile,
						varDir.entryList (QStringList ("*.xml"), QDir::Files))
					ParseFile (varDir.absoluteFilePath (logFile), result, data);
			}

			return result;
		}
	}

	void VacuumImportPage::SendImportHist (QStandardItem *accItem)
	{
		QVariantMap data = accItem->data (Roles::AccountData).toMap ();
		QString acc = data ["Jid"].toString ();
		const int pos = acc.indexOf ('@');
		if (acc <= 0)
			return;

		acc.replace (pos, 1, "_at_");
		acc.replace (0, pos, acc.left (pos).toUtf8 ().toPercentEncoding (QByteArray (), "_"));

		const QString& path = QDir::homePath () + "/.vacuum/archive/" + acc;
		qDebug () << Q_FUNC_INFO << data << path;

		QDir dir (path);
		Q_FOREACH (const QString& encJid, dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot))
		{
			const auto& hist = HandleJID (dir, encJid, data);
			if (hist.isEmpty ())
				continue;

			Entity e;
			e.Additional_ ["History"] = hist;
			e.Mime_ = "x-leechcraft/im-history-import";
			e.Additional_ ["AccountName"] = data ["Name"];
			e.Additional_ ["AccountID"] = data ["Jid"];
			e.Parameters_ = OnlyHandle | FromUserInitiated;

			emit gotEntity (e);
		}
	}
}
}
}
