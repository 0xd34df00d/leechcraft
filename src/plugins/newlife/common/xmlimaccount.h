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

#pragma once

#include <functional>
#include <QStringList>
#include <QVariantMap>

class QDomElement;
class QStandardItemModel;
class QStandardItem;

namespace LeechCraft
{
namespace NewLife
{
namespace Common
{
	class XMLIMAccount
	{
	public:
		struct ConfigAdapter
		{
			QStandardItemModel *Model_;
			QStringList ProfilesPath_;
			QString AccountsFileName_;

			std::function<QString (const QDomElement&)> Protocol_;
			std::function<QString (const QDomElement&)> Name_;
			std::function<bool (const QDomElement&)> IsEnabled_;
			std::function<QString (const QDomElement&)> JID_;
			std::function<void (const QDomElement&, QVariantMap&)> Additional_;
		};
	private:
		ConfigAdapter C_;
	public:
		XMLIMAccount (const ConfigAdapter&);
		void FindAccounts ();
	private:
		void ScanProfile (const QString& path, const QString& profileName);
		void ScanAccount (QStandardItem*, const QDomElement&);
	};
}
}
}
