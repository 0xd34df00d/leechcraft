#ifndef PLUGINS_NETWORKMONITOR_HEADERMODEL_H 
#define PLUGINS_NETWORKMONITOR_HEADERMODEL_H 
#include <QStandardItemModel>
#include <QNetworkAccessManager>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			class HeaderModel : public QStandardItemModel
			{
				Q_OBJECT
			public:
				HeaderModel (QObject* = 0);
				void AddHeader (const QString&, const QString&);
			};
		};
	};
};

#endif

