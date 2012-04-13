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

#include "otlozhu.h"
#include <QIcon>
#include "todotab.h"
#include "core.h"
#include "deltagenerator.h"

namespace LeechCraft
{
namespace Otlozhu
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Core::Instance ().SetProxy (proxy);

		connect (&Core::Instance (),
				SIGNAL (gotEntity (LeechCraft::Entity)),
				this,
				SIGNAL (gotEntity (LeechCraft::Entity)));

		TCTodo_ = TabClassInfo
		{
			GetUniqueID () + "_todo",
			GetName (),
			GetInfo (),
			GetIcon (),
			20,
			TFOpenableByRequest | TFSingle
		};
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Otlozhu";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Otlozhu";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple GTD-compatible ToDo manager.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/otlozhu/resources/images/otlozhu.svg");
	}

	TabClasses_t Plugin::GetTabClasses () const
	{
		return { TCTodo_ };
	}

	void Plugin::TabOpenRequested (const QByteArray& id)
	{
		if (id == TCTodo_.TabClass_)
		{
			auto tab = new TodoTab (TCTodo_, this);
			emit addNewTab (TCTodo_.VisibleName_, tab);
			emit raiseTab (tab);

			connect (tab,
					SIGNAL (removeTab (QWidget*)),
					this,
					SIGNAL (removeTab (QWidget*)));

			connect (tab,
					SIGNAL (gotEntity (LeechCraft::Entity)),
					this,
					SIGNAL (gotEntity (LeechCraft::Entity)));
		}
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown id"
					<< id;
	}

	Sync::ChainIDs_t Plugin::AvailableChains () const
	{
		Sync::ChainIDs_t result;
		result << GetUniqueID () + "_todos";
		return result;
	}

	Sync::Payloads_t Plugin::GetAllDeltas (const Sync::ChainID_t&) const
	{
		return Core::Instance ().GetDeltaGenerator ()->GetAllDeltas ();
	}

	Sync::Payloads_t Plugin::GetNewDeltas (const Sync::ChainID_t&) const
	{
		return Core::Instance ().GetDeltaGenerator ()->GetNewDeltas ();
	}

	void Plugin::PurgeNewDeltas (const Sync::ChainID_t&, quint32 num)
	{
		Core::Instance ().GetDeltaGenerator ()->PurgeDeltas (num);
	}

	void Plugin::ApplyDeltas (const Sync::Payloads_t& deltas, const Sync::ChainID_t&)
	{
		Core::Instance ().GetDeltaGenerator ()->Apply (deltas);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_otlozhu, LeechCraft::Otlozhu::Plugin);
