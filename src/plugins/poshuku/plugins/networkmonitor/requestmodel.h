#ifndef PLUGINS_POSHUKU_PLUGINS_REQUESTMODEL_H 
#define PLUGINS_POSHUKU_PLUGINS_REQUESTMODEL_H 
#include <QStandardItemModel>
#include <QNetworkAccessManager>

namespace LeechCraft
{
	namespace Poshuku
	{
		namespace Plugins
		{
			namespace NetworkMonitor
			{
				class RequestModel : public QStandardItemModel
				{
					Q_OBJECT
				public:
					RequestModel (QObject* = 0);
				public slots:
					void handleRequest (QNetworkAccessManager::Operation,
							const QNetworkRequest&, QNetworkReply*);
					void handleFinished ();
				};
			};
		};
	};
};

#endif

