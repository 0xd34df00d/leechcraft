#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H
#include <memory>
#include <QNetworkAccessManager>
#include <QTimer>

namespace LeechCraft
{
	class NetworkAccessManager : public QNetworkAccessManager
	{
		Q_OBJECT

		std::auto_ptr<QTimer> CookieSaveTimer_;
	public:
		NetworkAccessManager (QObject* = 0);
		virtual ~NetworkAccessManager ();
	protected:
		QNetworkReply* createRequest (Operation,
				const QNetworkRequest&, QIODevice*);
	private:
		void DoCommonAuth (const QString&, QAuthenticator*);
	private slots:
		void handleAuthentication (QNetworkReply*, QAuthenticator*);
		void handleProxyAuthentication (const QNetworkProxy&, QAuthenticator*);
		void handleSslErrors (QNetworkReply*, const QList<QSslError>&);
		void saveCookies () const;
	signals:
		void requestCreated (QNetworkAccessManager::Operation,
				const QNetworkRequest&, QNetworkReply*);
		void error (const QString&) const;
	};
};

#endif

