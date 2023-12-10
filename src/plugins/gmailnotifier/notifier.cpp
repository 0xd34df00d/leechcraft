/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notifier.h"
#include <QTextDocument>
#include <util/xpc/util.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"

namespace LC
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

		const int fullShow = XmlSettingsManager::Instance ().property ("ShowLastNMessages").toInt ();

		auto textWFallback = [] (const QString& text, const QString& fallback)
		{
			return text.isEmpty () ?
					fallback :
					text.toHtmlEscaped ();
		};

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
			result += QLocale {}.toString (info.Modified_, QLocale::LongFormat);
			result += "</p><p class=\"additionaltext\">";
			result += info.Summary_.toHtmlEscaped () + "</p>";

			if (++handledMsgs == fullShow)
				break;
		}

		if (infos.size () > fullShow)
			result += "<p><em>&hellip;" +
					tr ("and %1 more").arg (infos.size () - fullShow) +
					"</em></p>";

		const auto& e = Util::MakeNotification ("GMail", result, Priority::Info);
		Proxy_->GetEntityManager ()->HandleEntity (e);
	}
}
}
