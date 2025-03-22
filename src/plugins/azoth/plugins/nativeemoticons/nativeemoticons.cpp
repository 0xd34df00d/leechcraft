/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "nativeemoticons.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "nativeemoticonssource.h"
#include "kopeteemoticonssource.h"
#include "psiplusemoticonssource.h"

namespace LC
{
namespace Azoth
{
namespace NativeEmoticons
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		ResourceSources_ << new NativeEmoticonsSource ()
				<< new KopeteEmoticonsSource ()
				<< new PsiPlusEmoticonsSource ();
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.NativeEmoticons";
	}

	QString Plugin::GetName () const
	{
		return "Azoth NativeEmoticons";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Support for native Azoth emoticons packs as well as Kopete and Psi+ packs.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin";
		return result;
	}

	QList<QObject*> Plugin::GetResourceSources () const
	{
		return ResourceSources_;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_nativeemoticons, LC::Azoth::NativeEmoticons::Plugin);
