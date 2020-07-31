/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin <MaledictusDeMagog@gmail.com>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "vangog.h"
#include <QIcon>
#include <util/util.h>
#include "picasaservice.h"

namespace LC
{
namespace Blasq
{
namespace Vangog
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("blasq_vangog");
		Service_ = new PicasaService (proxy);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blasq.Vangog";
	}

	void Plugin::Release ()
	{
		Service_->Release ();
	}

	QString Plugin::GetName () const
	{
		return "Blasq Vangog";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Picasa support module for Blasq.");
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

LC_EXPORT_PLUGIN (leechcraft_blasq_vangog, LC::Blasq::Vangog::Plugin);
