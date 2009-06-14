#ifndef PLUGINS_CSTP_REPRESENTATIONMODEL_H
#define PLUGINS_CSTP_REPRESENTATIONMODEL_H
#include <QSortFilterProxyModel>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace CSTP
		{
			class RepresentationModel : public QSortFilterProxyModel
			{
				Q_OBJECT

			public:
				RepresentationModel (QObject* = 0);
				virtual ~RepresentationModel ();
			protected:
				virtual bool filterAcceptsColumn (int, const QModelIndex&) const;
			};
		};
	};
};

#endif

