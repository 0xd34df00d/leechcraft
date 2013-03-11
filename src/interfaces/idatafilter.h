/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include <QtPlugin>
#include <QByteArray>
#include <QString>
#include <QList>

/** @brief Base interface for data filter plugins.
 *
 * Data filter plugins provide some means to manipulate and alter data.
 * Examples of such plugins are image uploaders to various image bins,
 * text finders, etc.
 * 
 * The list of possible data filter variants (that is, image bins
 * supported by a single plugin) is returned from the GetFilterVariants()
 * function.
 *
 * Plugins implementing this interface are also expected to implement
 * IEntityHandler, considering (and accepting) entities with MIME
 * "x-leechcraft/data-filter-request". Such entities will contain the
 * entity to filter (like, a piece of text or an image) in the
 * Entity::Entity_ field and may contain the "DataFilter" key in the
 * Entity::Additional_ map with the name of the exact filter variant
 * to use (if user has already selected it).
 */
class Q_DECL_EXPORT IDataFilter
{
public:
	struct FilterVariant
	{
		QByteArray ID_;
		QString Name_;
		QString Description_;
	};

	virtual ~IDataFilter () {}

	virtual QString GetFilterVerb () const = 0;

	virtual QList<FilterVariant> GetFilterVariants () const = 0;
};

Q_DECLARE_INTERFACE (IDataFilter, "org.Deviant.LeechCraft.IDataFilter/1.0");
