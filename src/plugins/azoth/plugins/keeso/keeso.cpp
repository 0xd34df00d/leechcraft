/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "keeso.h"
#include <QIcon>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace Azoth
{
namespace Keeso
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Commands_.append ({
				{ "/keeso" },
				[] (ICLEntry*, QString& text) -> bool
				{
					text = text.mid (QString ("/keeso ").length ()).trimmed ();
					bool isUpper = qrand () % 2;
					for (int i = 0, length = text.length (); i < length; ++i)
					{
						const auto& c = text.at (i);
						const auto& u = c.toUpper ();
						const auto& l = c.toLower ();
						if (u == l)
							continue;

						text [i] = isUpper ? u : l;
						isUpper = (qrand () % 4) ? !isUpper : isUpper;
					}

					return false;
				},
				tr ("Randomily changes the capitalization of outbound messages."),
				{}
			});
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Keeso";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Keeso";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to WrItE lIkE tHiS easily.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	StaticCommands_t Plugin::GetStaticCommands (ICLEntry*)
	{
		return Commands_;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_keeso, LC::Azoth::Keeso::Plugin);
