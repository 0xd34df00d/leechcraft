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
			
			void JobHolderRepresentation::SelectionChanged (const QModelIndex& index)
			{
				if (index.isValid ())
					Selected_ = mapToSource (index);
				else
					Selected_ = QModelIndex ();
				invalidateFilter ();
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

