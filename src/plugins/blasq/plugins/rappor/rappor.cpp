/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rappor.h"
#include <QIcon>
#include <util/util.h>
#include "vkservice.h"

namespace LC
{
namespace Blasq
{
namespace Rappor
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("blasq_rappor");

		Service_ = new VkService (proxy);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blasq.Rappor";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Blasq Rappor";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("VKontakte support module for Blasq.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Blasq.ServicePlugin";
		return result;
	}

	QList<IService*> Plugin::GetServices () const
	{
		return { Service_ };
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_blasq_rappor, LC::Blasq::Rappor::Plugin);
