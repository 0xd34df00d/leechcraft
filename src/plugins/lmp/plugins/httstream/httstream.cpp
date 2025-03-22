/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "httstream.h"
#include "httpstreamfilter.h"

namespace LC
{
namespace LMP
{
namespace HttStream
{
	void Plugin::Init (ICoreProxy_ptr)
	{
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.HttStream";
	}

	QString Plugin::GetName () const
	{
		return "LMP HttStream";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Adds support for streaming music played in LMP via HTTP.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.FiltersProvider";
		return result;
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr)
	{
	}

	QList<EffectInfo> Plugin::GetEffects () const
	{
		return
		{
			{
				GetUniqueID () + ".Filter",
				tr ("HTTP streaming"),
				{},
				false,
				[this] (const QByteArray& instance, IPath *path) -> IFilterElement*
				{
					return new HttpStreamFilter
						{
							GetUniqueID () + ".Filter",
							instance,
							path
						};
				}
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_httstream, LC::LMP::HttStream::Plugin);
