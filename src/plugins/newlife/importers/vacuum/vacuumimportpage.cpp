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
#include <QDir>

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

	void VacuumImportPage::handleAccepted ()
	{
	}
}
}
}
