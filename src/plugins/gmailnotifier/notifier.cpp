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

#include "notifier.h"
#include <util/util.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace GmailNotifier
{
	Notifier::Notifier (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
	}

	void Notifier::notifyAbout (const ConvInfos_t& infos)
	{
		qDebug () << Q_FUNC_INFO << infos.size ();
		if (infos == PreviousInfos_)
			return;

		PreviousInfos_ = infos;

		if (infos.isEmpty ())
			return;

		const int fullShow = XmlSettingsManager::Instance ()->
				property ("ShowLastNMessages").toInt ();

		auto textWFallback = [] (const QString& text, const QString& fallback)
			{ return text.isEmpty () ? fallback : text; };

		int handledMsgs = 0;
		QString result;
		for (const auto& info : infos)
		{
			result += QString::fromUtf8 ("<p><font color=\"#004C00\">\302\273</font>&nbsp;<a href=\"");
			result += info.Link_.toString () + "\">";
			result += textWFallback (info.Title_, tr ("No subject")) + "</a> " + tr ("from") + " ";
			result += "<a href=\"https://mail.google.com/mail?extsrc=mailto&url=mailto:";
			result += info.AuthorEmail_ + "\">";
			result += info.AuthorName_ + "</a><br/>";
			result += tr ("at") + " ";
			result += info.Modified_.toString (Qt::SystemLocaleLongDate);
			result += "</p><p class=\"additionaltext\">";
			result += info.Summary_ + "</p>";

			if (++handledMsgs == fullShow)
				break;
		}

		if (infos.size () > fullShow)
			result += "<p><em>&hellip;" +
					tr ("and %1 more").arg (infos.size () - fullShow) +
					"</em></p>";

		const auto& e = Util::MakeNotification ("GMail", result, PInfo_);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
