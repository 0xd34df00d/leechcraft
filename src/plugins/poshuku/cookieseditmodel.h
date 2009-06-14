#ifndef PLUGINS_POSHUKU_COOKIESEDITMODEL_H
#define PLUGINS_POSHUKU_COOKIESEDITMODEL_H
#include <QStandardItemModel>
#include <QNetworkCookie>
#include <QMap>

namespace LeechCraft
{
	namespace Util
	{
		class CustomCookieJar;
	};

	namespace Plugins
	{
		namespace Poshuku
		{
			class CookiesEditModel : public QStandardItemModel
			{
				Q_OBJECT

				Util::CustomCookieJar *Jar_;
				QMap<int, QNetworkCookie> Cookies_;
			public:
				CookiesEditModel (QObject* = 0);
				QNetworkCookie GetCookie (const QModelIndex&) const;
				void SetCookie (const QModelIndex&, const QNetworkCookie&);
				void RemoveCookie (const QModelIndex&);
			private:
				void AddCookie (const QNetworkCookie&);
			};
		};
	};
};

#endif

