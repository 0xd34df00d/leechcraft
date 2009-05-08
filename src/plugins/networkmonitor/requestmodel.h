#ifndef PLUGINS_REQUESTMODEL_H 
#define PLUGINS_REQUESTMODEL_H 
#include <QStandardItemModel>
#include <QNetworkAccessManager>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			class HeaderModel;

			class RequestModel : public QStandardItemModel
			{
				Q_OBJECT

				HeaderModel *RequestHeadersModel_;
				HeaderModel *ReplyHeadersModel_;
				bool Clear_;
			public:
				RequestModel (QObject* = 0);
				HeaderModel* GetRequestHeadersModel () const;
				HeaderModel* GetReplyHeadersModel () const;
			public slots:
				void handleRequest (QNetworkAccessManager::Operation,
						const QNetworkRequest&, QNetworkReply*);
				void handleFinished ();
				void setClear (bool);
			private slots:
				void handleCurrentChanged (const QModelIndex&);
			};
		};
	};
};

#endif

