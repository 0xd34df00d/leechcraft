#ifndef PLUGINS_BITTORRENT_REPRESENTATIONMODEL_H
#define PLUGINS_BITTORRENT_REPRESENTATIONMODEL_H
#include <QSortFilterProxyModel>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace BitTorrent
		{
			class RepresentationModel : public QSortFilterProxyModel
			{
				Q_OBJECT

			public:
				RepresentationModel (QObject* = 0);
				virtual ~RepresentationModel ();
			};
		};
	};
};

#endif

