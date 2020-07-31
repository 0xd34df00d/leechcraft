/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "potorchu.h"
#include <QIcon>
#include <util/util.h>
#include "visualfilter.h"

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("lmp_potorchu");
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.Potorchu";
	}

	QString Plugin::GetName () const
	{
		return "LMP Potorchu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Visualization effects for LMP.");
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

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LmpProxy_ = proxy;
	}

	QList<EffectInfo> Plugin::GetEffects () const
	{
		return
		{
			{
				GetUniqueID () + ".Filter",
				tr ("Visual effects"),
				{},
				false,
				[this] (const QByteArray&, IPath*) -> IFilterElement*
				{
					return new VisualFilter
						{
							GetUniqueID () + ".Filter",
							LmpProxy_
						};
				}
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_potorchu, LC::LMP::Potorchu::Plugin);
