/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_INTERFACES_ISUPPORTRIEX_H
#define PLUGINS_AZOTH_INTERFACES_ISUPPORTRIEX_H
#include <QFlags>
#include <QMetaType>
#include <QStringList>

namespace LeechCraft
{
namespace Azoth
{
	struct RIEXItem
	{
		enum Action
		{
			AAdd,
			ADelete,
			AModify
		} Action_;

		QString ID_;
		QString Nick_;
		QStringList Groups_;
	};

	/** @brief Interface representing Roster Item Exchange-like things.
	 *
	 * This interface should be implemented by account objects that
	 * support exchanging contact list items between different users.
	 *
	 * This interface is modeled after XEP-0144.
	 */
	class ISupportRIEX
	{
	public:
		virtual ~ISupportRIEX () {}

		virtual void riexItemsSuggested (QList<LeechCraft::Azoth::RIEXItem> items,
				QObject *from, QString message) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Azoth::ISupportRIEX,
		"org.Deviant.LeechCraft.Azoth.ISupportRIEX/1.0");
Q_DECLARE_METATYPE (LeechCraft::Azoth::RIEXItem);

#endif
