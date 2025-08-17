/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cookieseditmodel.h"
#include <stdexcept>
#include <algorithm>
#include <QtDebug>
#include <QNetworkAccessManager>
#include <QString>
#include <QtGlobal>
#include <util/network/customcookiejar.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
	using LC::Util::CustomCookieJar;

	CookiesEditModel::CookiesEditModel (QObject *parent)
	: QStandardItemModel (parent)
	{
		setHorizontalHeaderLabels (QStringList (tr ("Domain (cookie name)")));
		Jar_ = qobject_cast<CustomCookieJar*> (Core::Instance ()
					.GetNetworkAccessManager ()->cookieJar ());

		auto cookies = Jar_->allCookies ();
		std::stable_sort (cookies.begin (), cookies.end (),
				[] (const QNetworkCookie& c1, const QNetworkCookie& c2)
					{ return c1.domain () < c2.domain (); });
		int idx = 0;
		for (const auto& cookie : cookies)
			Cookies_ [idx++] = cookie;

		for (int i = 0; i < Cookies_.size (); ++i)
		{
			QString domain = Cookies_ [i].domain ();

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
			QStandardItem *item = new QStandardItem (QString (Cookies_ [i].name ()));
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
				return Cookies_ [i];
		}
	}

	void CookiesEditModel::SetCookie (const QModelIndex& index,
			const QNetworkCookie& cookie)
	{
		if (index.isValid ())
		{
			int i = itemFromIndex (index)->data ().toInt ();
			if (i == -1)
				AddCookie (cookie);
			else
			{
				Cookies_ [i] = cookie;
				emit itemChanged (itemFromIndex (index));
			}
		}
		else
			AddCookie (cookie);

		Jar_->setAllCookies (Cookies_.values ());
	}

	void CookiesEditModel::RemoveCookie (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		QStandardItem *item = itemFromIndex (index);
		int i = item->data ().toInt ();
		if (i == -1)
		{
			for (int j = 0; j < item->rowCount (); ++j)
			{
				Cookies_.remove (item->child (j)->data ().toInt ());
			}
			qDeleteAll (takeRow (item->row ()));
		}
		else
		{
			Cookies_.remove (i);
			qDeleteAll (item->parent ()->takeRow (item->row ()));
		}
		Jar_->setAllCookies (Cookies_.values ());
	}

	void CookiesEditModel::AddCookie (const QNetworkCookie& cookie)
	{
		int i = 0;
		if (!Cookies_.isEmpty ())
			i = Cookies_.lastKey () + 1;
		Cookies_ [i] = cookie;

		QString domain = cookie.domain ();

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
		QStandardItem *item = new QStandardItem (QString (Cookies_ [i].name ()));
		item->setData (i);
		item->setEditable (false);
		parent->appendRow (item);

		Jar_->setAllCookies (Cookies_.values ());
	}
}
}
