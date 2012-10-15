/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "pogooglue.h"
#include <QIcon>
#include <QUrl>
#include <util/util.h>
#include <interfaces/entitytesthandleresult.h>

namespace LeechCraft
{
namespace Pogooglue
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("pogooglue");
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Pogooglue";
	}

	QString Plugin::GetName () const
	{
		return "Pogooglue";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to search for selected text in Google in two clicks.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& e) const
	{
		if (e.Mime_ != "x-leechcraft/data-filter-request" ||
				!e.Entity_.canConvert<QString> ())
			return EntityTestHandleResult ();

		if (e.Additional_.contains ("DataFilter"))
		{
			const auto& rawCat = e.Additional_ ["DataFilter"].toByteArray ();
			const auto& catStr = QString::fromUtf8 (rawCat.data ());
			const auto& vars = GetFilterVariants ();
			if (std::find_if (vars.begin (), vars.end (),
					[&catStr] (decltype (vars.front ()) var)
						{ return var.ID_ == catStr; }) == vars.end ())
				return EntityTestHandleResult ();
		}

		const auto& str = e.Entity_.toString ();
		return str.size () < 200 && str.count ("\n") < 3 ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity e)
	{
		const auto& str = e.Entity_.toString ();
		GoogleIt (str);
	}

	QString Plugin::GetFilterVerb () const
	{
		return tr ("Google it!");
	}

	QList<IDataFilter::FilterVariant> Plugin::GetFilterVariants () const
	{
		return { { GetUniqueID () + "_Google", "Google", "Google" } };
	}

	void Plugin::GoogleIt (QString text)
	{
		QString withoutPercent = text;
		withoutPercent.remove (QRegExp ("%%??",
				Qt::CaseInsensitive, QRegExp::Wildcard));
		QUrl testUrl (withoutPercent);
		QUrl result;
		if (testUrl.toString () == withoutPercent)
			result = QUrl::fromEncoded (text.toUtf8 ());
		else
			result = QUrl (text);

		if (result.scheme ().isEmpty ())
		{
			if (!text.count (' ') && text.count ('.'))
				result = QUrl (QString ("http://") + text);
			else
			{
				text.replace ('+', "%2B");
				text.replace (' ', '+');
				QString urlStr = QString ("http://www.google.com/search?q=%2"
						"&client=leechcraft_poshuku"
						"&ie=utf-8"
						"&rls=org.leechcraft:%1")
					.arg (QLocale::system ().name ().replace ('_', '-'))
					.arg (text);
				result = QUrl::fromEncoded (urlStr.toUtf8 ());
			}
		}

		const auto& e = Util::MakeEntity (result,
				QString (),
				LeechCraft::FromUserInitiated | LeechCraft::OnlyHandle);
		emit gotEntity (e);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_pogooglue, LeechCraft::Pogooglue::Plugin);