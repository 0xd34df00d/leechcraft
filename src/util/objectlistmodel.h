/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Andrey Batyiev
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

#ifndef LEECHCRAFT_UTIL_OBJECTLISTMODEL_H
#define LEECHCRAFT_UTIL_OBJECTLISTMODEL_H
#include <QAbstractListModel>
#include <QList>
#include "utilconfig.h"

namespace LeechCraft
{
namespace Util
{
	/**
	 * Small wrapper around QList<QObject*> to provide better integration
	 * with QML data models.
	 */
	class UTIL_API ObjectListModel : public QAbstractListModel
	{
		Q_OBJECT

		QList<QObject*> Items_;
	public:
		enum DataRoles
		{
	         ObjectRole = Qt::UserRole + 1,
		};
		ObjectListModel (QObject * = 0);

		// QAbstractListModel redefinitions
		virtual QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;

		/**
		 * Append object into list.
		 */
		void Append (QObject *);

		/**
		 * Insert object at first position in list.
		 */
		void Prepend (QObject *);

		/**
		 * Remove object from list.
		 */
		void Remove (QObject *);

		/**
		 * Remove object at position from list.
		 */
		void Remove (int position);

		/**
		 * Set objects as whole.
		 */
		void SetObjects (const QList<QObject*>&);

		/**
		 * Get objects as whole.
		 */
		QList<QObject*> GetObjects () const;
	};
}
}

#endif // LEECHCRAFT_UTIL_OBJECTLISTMODEL_H
