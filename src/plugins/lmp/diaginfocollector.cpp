/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "diaginfocollector.h"
#include <gst/gst.h>
#include <taglib/taglib.h>
#include <util/sll/qtutil.h>

namespace LC::LMP
{
	QString CollectDiagInfo ()
	{
		QStringList strs
		{
			QStringLiteral ("Built with GStreamer %1.%2.%3; running with %4")
					.arg (GST_VERSION_MAJOR)
					.arg (GST_VERSION_MINOR)
					.arg (GST_VERSION_MICRO)
					.arg (QString::fromUtf8 (gst_version_string ())),
#ifdef WITH_LIBGUESS
			QStringLiteral ("Built WITH libguess"),
#else
			QStringLiteral ("Built WITHOUT libguess"),
#endif
			QStringLiteral ("Built with Taglib %1.%2.%3")
					.arg (TAGLIB_MAJOR_VERSION)
					.arg (TAGLIB_MINOR_VERSION)
					.arg (TAGLIB_PATCH_VERSION),
			QStringLiteral ("GStreamer plugins:"),
		};

		const auto plugins = gst_registry_get_plugin_list (gst_registry_get ());
		auto node = plugins;
		QStringList pluginsList;
		while (node)
		{
			const auto plugin = static_cast<GstPlugin*> (node->data);
			pluginsList << u"* %1 (from %2)"_qsv
					.arg (QLatin1String { gst_plugin_get_name (plugin) },
						  QLatin1String { gst_plugin_get_filename (plugin) });

			node = g_list_next (node);
		}
		pluginsList.sort ();

		strs << pluginsList;

		gst_plugin_list_free (plugins);

		return strs.join ('\n');
	}
}
