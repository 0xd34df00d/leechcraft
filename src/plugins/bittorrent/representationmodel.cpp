#include "representationmodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			RepresentationModel::RepresentationModel (QObject *parent)
			: QSortFilterProxyModel (parent)
			{
				setObjectName ("Torrent RepresentationModel");
			}
			
			RepresentationModel::~RepresentationModel ()
			{
			}
			
		};
	};
};

