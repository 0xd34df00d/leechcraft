/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "deathnote.h"
#include <QIcon>
#include <util/util.h>
#include "fotobilderservice.h"

namespace LC
{
namespace Blasq
{
namespace DeathNote
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("blasq_deathnote");

		Service_ = new FotoBilderService (proxy);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blasq.DeathNote";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Blasq DeathNote";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("LiveJournal FotoBilder support module for Blasq.");
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

LC_EXPORT_PLUGIN (leechcraft_blasq_deathnote, LC::Blasq::DeathNote::Plugin);
