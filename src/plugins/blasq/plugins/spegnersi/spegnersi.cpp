/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "spegnersi.h"
#include <QIcon>
#include "flickrservice.h"

namespace LC
{
namespace Blasq
{
namespace Spegnersi
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Service_ = new FlickrService (proxy);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Blasq.Spegnersi";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Blasq Spegnersi";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Flickr support module for Blasq.");
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

LC_EXPORT_PLUGIN (leechcraft_blasq_spegnersi, LC::Blasq::Spegnersi::Plugin);
