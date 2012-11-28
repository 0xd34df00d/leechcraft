/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include <QMetaType>
#include <QVariant>

namespace LeechCraft
{
namespace Blogique
{
	/** @brief Interface representing a side widget with main post options'.
	*
	**/
	class IPostOptionsWidget
	{
	public:
		virtual ~IPostOptionsWidget () {};

		/** @brief Returns list of tags for entry.
		*
		* @return List of tags
		**/
		virtual QStringList GetTags () const = 0;

		/** @brief Set tags.
		 * 
		 **/
		virtual void SetTags (const QStringList& tags) = 0;

		/** @brief Returns date when post was written.
		 * 
		 * @return Post date
		 **/
		virtual QDateTime GetPostDate () const = 0;

		/** @brief Set post timestamp.
		 * 
		 **/
		virtual void SetPostDate (const QDateTime& dt) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LeechCraft::Blogique::IPostOptionsWidget,
		"org.Deviant.LeechCraft.Blogique.IPostOptionsWidget/1.0");
