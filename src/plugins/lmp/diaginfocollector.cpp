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

namespace LC
{
namespace LMP
{
	DiagInfoCollector::DiagInfoCollector ()
	{
		Strs_ << QString { "Built with GStreamer %1.%2.%3; running with %4" }
				.arg (GST_VERSION_MAJOR)
				.arg (GST_VERSION_MINOR)
				.arg (GST_VERSION_MICRO)
				.arg (QString::fromUtf8 (gst_version_string ()));
#ifdef WITH_LIBGUESS
		Strs_ << "Built WITH libguess";
#else
		Strs_ << "Built WITHOUT libguess";
#endif
		Strs_ << QString { "Built with Taglib %1.%2.%3" }
				.arg (TAGLIB_MAJOR_VERSION)
				.arg (TAGLIB_MINOR_VERSION)
				.arg (TAGLIB_PATCH_VERSION);
		Strs_ << "GStreamer plugins:";

		const auto plugins = gst_registry_get_plugin_list (gst_registry_get ());
		auto node = plugins;
		QStringList pluginsList;
		while (node)
		{
			const auto plugin = static_cast<GstPlugin*> (node->data);
			pluginsList << QString { "* %1 (from %2)" }
					.arg (QString::fromUtf8 (gst_plugin_get_name (plugin)))
					.arg (QString::fromUtf8 (gst_plugin_get_filename (plugin)));

			node = g_list_next (node);
		}

		pluginsList.sort ();

		Strs_ += pluginsList;

		gst_plugin_list_free (plugins);
	}

	QString DiagInfoCollector::operator() () const
	{
		return Strs_.join ("\n");
	}
}
}
