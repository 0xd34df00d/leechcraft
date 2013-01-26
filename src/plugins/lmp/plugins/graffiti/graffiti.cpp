/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "graffiti.h"
#include <QIcon>
#include <util/util.h>
#include "graffititab.h"

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator ("lmp_graffiti");

		TaggerTC_ =
		{
			GetUniqueID () + "_Tagger",
			"LMP Graffiti",
			GetInfo (),
			GetIcon (),
			0,
			TFOpenableByRequest
		};
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.LMP.Graffiti";
	}

	QString Plugin::GetName () const
	{
		return "LMP Graffiti";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Allows one to manipulate audio files tags.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.LMP.General";
		return result;
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TaggerTC_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& tabClass)
	{
		if (TaggerTC_.TabClass_ == tabClass)
		{
			auto tab = new GraffitiTab (LMPProxy_, TaggerTC_, this);
			emit addNewTab (TaggerTC_.VisibleName_, tab);
			emit raiseTab (tab);

			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown tab class"
					<< tabClass;
	}

	void Plugin::SetLMPProxy (ILMPProxy_ptr proxy)
	{
		LMPProxy_ = proxy;
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_lmp_graffiti, LeechCraft::LMP::Graffiti::Plugin);
