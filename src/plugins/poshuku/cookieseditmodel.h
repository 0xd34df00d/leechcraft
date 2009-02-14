#ifndef COOKIESEDITMODEL_H
#define COOKIESEDITMODEL_H
#include <QStandardItemModel>
#include <QNetworkCookie>

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
	QList<QNetworkCookie> Cookies_;
public:
	CookiesEditModel (QObject* = 0);
	QNetworkCookie GetCookie (const QModelIndex&) const;
	void SetCookie (const QModelIndex&, const QNetworkCookie&);
};

#endif

