/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "fradj.h"
#include <util/util.h>
#include "eq10bandeffect.h"

namespace LC
{
namespace LMP
{
namespace Fradj
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("lmp_fradj");
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.Fradj";
	}

	QString Plugin::GetName () const
	{
		return "LMP FrAdj";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Configurable equalizer effect for LMP.");
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
				GetUniqueID () + ".10Band",
				tr ("10-band equalizer"),
				{},
				true,
				[this] (const QByteArray&, IPath*) -> IFilterElement*
				{
					return new Eq10BandEffect { GetUniqueID () + ".10Band" };
				}
			}
		};
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_httstream, LC::LMP::Fradj::Plugin);
