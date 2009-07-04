#include "representationmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			RepresentationModel::RepresentationModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
				setObjectName ("CSTP RepresentationModel");
			}
			
			RepresentationModel::~RepresentationModel ()
			{
			}
			
			bool RepresentationModel::filterAcceptsColumn (int column, const QModelIndex&) const
			{
				if (column > 3)
					return false;
				else
					return true;
			}
		};
	};
};

