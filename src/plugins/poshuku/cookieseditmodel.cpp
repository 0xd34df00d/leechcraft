#include "cookieseditmodel.h"
#include <stdexcept>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <QtDebug>
#include <QNetworkAccessManager>
#include <QString>
#include <QtGlobal>
#include <plugininterface/customcookiejar.h>
#include "core.h"

using LeechCraft::Util::CustomCookieJar;

CookiesEditModel::CookiesEditModel (QObject *parent)
: QStandardItemModel (parent)
{
	setHorizontalHeaderLabels (QStringList (tr ("Domain (cookie name)")));
	Jar_ = qobject_cast<CustomCookieJar*> (Core::Instance ()
				.GetNetworkAccessManager ()->cookieJar ());

	Cookies_ = Jar_->allCookies ();

	typedef boost::function<QString (const QNetworkCookie&)> name_t;
	std::stable_sort (Cookies_.begin (), Cookies_.end (),
			boost::bind (std::less<QString> (),
				boost::bind<QString> (name_t (&QNetworkCookie::domain), _1),
				boost::bind<QString> (name_t (&QNetworkCookie::domain), _2)));

	for (int i = 0; i < Cookies_.size (); ++i)
	{
		QString domain = Cookies_.at (i).domain ();

		QList<QStandardItem*> foundItems = findItems (domain);
		QStandardItem *parent = 0;
		if (!foundItems.size ())
		{
			parent = new QStandardItem (domain);
			parent->setEditable (false);
			parent->setData (-1);
			invisibleRootItem ()->appendRow (parent);
		}
		else
			parent = foundItems.back ();
		QStandardItem *item = new QStandardItem (QString (Cookies_.at (i).name ()));
		item->setData (i);
		item->setEditable (false);
		parent->appendRow (item);
	}
}

QNetworkCookie CookiesEditModel::GetCookie (const QModelIndex& index) const
{
	if (!index.isValid ())
		return QNetworkCookie ();
	else
	{
		int i = itemFromIndex (index)->data ().toInt ();
		if (i == -1)
			throw std::runtime_error ("Wrong index");
		else
			return Cookies_.at (i);
	}
}

void CookiesEditModel::SetCookie (const QModelIndex& index, 
		const QNetworkCookie& cookie)
{
	if (index.isValid ())
	{
		int i = itemFromIndex (index)->data ().toInt ();
		if (i == -1)
			Cookies_.push_back (cookie);
		else
			Cookies_ [i] = cookie;
	}
	else
		Cookies_.push_back (cookie);

	Jar_->setAllCookies (Cookies_);
}

