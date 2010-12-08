/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGININTERFACE_RESOURCELOADER_H
#define PLUGININTERFACE_RESOURCELOADER_H
#include <boost/shared_ptr.hpp>
#include <QObject>
#include <QStringList>
#include <QDir>
#include "piconfig.h"

class QAbstractItemModel;
class QStandardItemModel;
class QSortFilterProxyModel;

namespace LeechCraft
{
	namespace Util
	{
		typedef boost::shared_ptr<QIODevice> QIODevice_ptr;

		class PLUGININTERFACE_API ResourceLoader : public QObject
		{
			Q_OBJECT

			QStringList LocalPrefixesChain_;
			QStringList GlobalPrefixesChain_;
			QString RelativePath_;

			QStandardItemModel *SubElemModel_;
			QStringList NameFilters_;
			QDir::Filters AttrFilters_;
			QSortFilterProxyModel *SortModel_;
		public:
			ResourceLoader (const QString& relPath, QObject* = 0);

			void AddGlobalPrefix (QString prefix);
			void AddLocalPrefix (QString prefix = QString ());

			QString GetPath (const QStringList& pathVariants) const;
			QIODevice_ptr Load (const QStringList& pathVariants) const;

			QAbstractItemModel* GetSubElemModel () const;

			void SetAttrFilters (QDir::Filters);
			void SetNameFilters (const QStringList&);
		private:
			void ScanPath (const QString&);
		};
	}
}

#endif
