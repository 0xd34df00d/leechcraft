/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "flgetter.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QtDebug>
#include "account.h"
#include "authmanager.h"
#include "urls.h"
#include "common.h"

namespace LeechCraft
{
namespace NetStoreManager
{
namespace YandexDisk
{
	FLGetter::FLGetter (Account *acc)
	: ActorBase (acc)
	{
	}

	QNetworkReply* FLGetter::MakeRequest ()
	{
		emit statusChanged (tr ("Getting filelist..."));
		return Mgr_->get (A_->MakeRequest (QUrl ("http://narod.yandex.ru/disk/all/page1/?sort=cdate%20desc")));
	}

	void FLGetter::HandleReply (QNetworkReply *reply)
	{
		QString page = reply->readAll ();
		if (page.isEmpty ())
		{
			A_->GetAuthManager ()->	GetCookiesFor (A_->GetLogin (), A_->GetPassword (), true);
			return;
		}

		page.replace ("<wbr/>", "");

		QList<FLItem> items;

		QRegExp rx ("class=\"\\S+icon\\s(\\S+)\"[^<]+<img[^<]+</i[^<]+</td[^<]+<td[^<]+<input[^v]+value=\"(\\d+)\" data-token=\"(\\S+)\""
				"[^<]+</td[^<]+<td[^<]+<span\\sclass='b-fname'><a\\shref=\"(\\S+)\">([^<]+)</a>.*"
				"<td class=\"size\">(\\S+)</td>.*data-token=\"(\\S+)\".*<i class=\"([^\"]+)\".*"
				"<td class=\"date prolongate\"><nobr>(\\S+ \\S+)</nobr></td>");
		rx.setMinimal (true);
		int cpos = rx.indexIn (page);
		while (cpos != -1)
		{
			FLItem item =
			{
				QString::fromUtf8 (rx.cap (5).toLatin1 ()),
				rx.cap (2),
				rx.cap (3),
				rx.cap (4),
				GetIconForType (rx.cap (1)),
				QString::fromUtf8 (rx.cap (6).toLatin1 ()).replace ("&nbsp;", " "),
				rx.cap (7),
				QString::fromUtf8 (rx.cap (9).toLatin1 ()),
				rx.cap (8) == "b-old-icon b-old-icon-pwd-on"
			};

			cpos = rx.indexIn (page, cpos + 1);

			items << item;
		}
		emit gotFiles (items);

		QRegExp rxnp ("<a\\sid=\"next_page\"\\shref=\"([^\"]+)\"");
		cpos = rxnp.indexIn (page);
		if (cpos > 0 && !rxnp.cap (1).isEmpty ())
		{
			QNetworkRequest nr = A_->MakeRequest ("http://narod.yandex.ru"+rxnp.cap(1));
			Mgr_->get (nr);
		}
		else
		{
			emit statusChanged (tr ("Filelist downloaded"));
			emit finished ();
		}
	}
}
}
}
