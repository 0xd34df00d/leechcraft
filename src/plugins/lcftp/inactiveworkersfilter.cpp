#include "inactiveworkersfilter.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			InactiveWorkersFilter::InactiveWorkersFilter (Core *parent)
			: QSortFilterProxyModel (parent)
			, Core_ (parent)
			{
				setSourceModel (Core_);
				setDynamicSortFilter (true);
			}

			bool InactiveWorkersFilter::filterAcceptsRow (int row, const QModelIndex&) const
			{
				return Core_->IsAcceptable (row);
			}
		};
	};
};

