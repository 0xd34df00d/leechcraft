/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QStringList>
#include <QVariantMap>

class QDomElement;
class QStandardItemModel;
class QStandardItem;

namespace LC
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
