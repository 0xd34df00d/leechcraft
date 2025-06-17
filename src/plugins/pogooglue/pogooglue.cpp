/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pogooglue.h"
#include <QIcon>
#include <QUrl>
#include <util/xpc/util.h>
#include <interfaces/entitytesthandleresult.h>
#include <interfaces/core/ientitymanager.h>

namespace LC
{
namespace Pogooglue
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
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
			return {};

		if (e.Additional_.contains ("DataFilter"))
		{
			const auto& rawCat = e.Additional_ ["DataFilter"].toByteArray ();
			const auto& catStr = QString::fromUtf8 (rawCat.data ());
			const auto& vars = GetFilterVariants (e.Entity_);
			if (std::none_of (vars.begin (), vars.end (),
					[&catStr] (const auto& var) { return var.ID_ == catStr; }))
				return {};
		}

		if (e.Entity_.typeId () != QMetaType::QString)
			return {};

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

	QList<IDataFilter::FilterVariant> Plugin::GetFilterVariants (const QVariant&) const
	{
		return { { GetUniqueID () + "_Google", "Google", "Google", {} } };
	}

	void Plugin::GoogleIt (const QString& text)
	{
		const auto& urlStr = QString ("http://www.google.com/search?q=%2"
				"&client=leechcraft_poshuku"
				"&ie=utf-8"
				"&rls=org.leechcraft:%1")
			.arg (QLocale::system ().name ().replace ('_', '-'))
			.arg (QString::fromUtf8 (QUrl::toPercentEncoding (text)));
		const auto& result = QUrl::fromEncoded (urlStr.toUtf8 ());

		Proxy_->GetEntityManager ()->HandleEntity (Util::MakeEntity (result,
					{},
					FromUserInitiated | OnlyHandle));
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_pogooglue, LC::Pogooglue::Plugin);
