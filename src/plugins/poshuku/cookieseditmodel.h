#ifndef COOKIESEDITMODEL_H
#define COOKIESEDITMODEL_H
#include <QStandardItemModel>
#include <QNetworkCookie>
#include <QMap>

namespace LeechCraft
{
	namespace Util
	{
		class CustomCookieJar;
	};
};

class CookiesEditModel : public QStandardItemModel
{
	Q_OBJECT

	LeechCraft::Util::CustomCookieJar *Jar_;
	QMap<int, QNetworkCookie> Cookies_;
public:
	CookiesEditModel (QObject* = 0);
	QNetworkCookie GetCookie (const QModelIndex&) const;
	void SetCookie (const QModelIndex&, const QNetworkCookie&);
	void RemoveCookie (const QModelIndex&);
private:
	void AddCookie (const QNetworkCookie&);
};

#endif

