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

#include "jobholderrepresentation.h"
#include <QTimer>
#include <QtDebug>
#include "aggregator.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Aggregator
		{
			JobHolderRepresentation::JobHolderRepresentation (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
				setDynamicSortFilter (true);
				XmlSettingsManager::Instance ()->RegisterObject ("ShowInDashboard",
						this,
						"invalidate");
			}
			
			QModelIndex JobHolderRepresentation::SelectionChanged (const QModelIndex& index)
			{
				if (index.isValid ())
					Selected_ = mapToSource (index);
				else
					Selected_ = QModelIndex ();
				invalidateFilter ();
				return mapFromSource (Selected_);
			}
			
			bool JobHolderRepresentation::filterAcceptsRow (int row,
					const QModelIndex&) const
			{
					// The row won't show up anyway in the job list if it was empty, so
					// we can just check if it has unread items or selected. Later means
					// that user's just clicked last unread item there.
				if (XmlSettingsManager::Instance ()->property ("ShowInDashboard").toBool ())
					return sourceModel ()->index (row, 1).data ().toInt () ||
						(Selected_.isValid () ? row == Selected_.row () : false);
				else
					return false;
			}
		};
	};
};

