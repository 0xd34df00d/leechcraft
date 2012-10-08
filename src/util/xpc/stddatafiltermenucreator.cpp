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

#include "stddatafiltermenucreator.h"
#include <QVariant>
#include <QMenu>
#include <interfaces/iinfo.h>
#include <interfaces/idatafilter.h>
#include <interfaces/core/ientitymanager.h>
#include "../util.h"

namespace LeechCraft
{
namespace Util
{
	namespace
	{
		struct DataFilterActionInfo
		{
			Entity Entity_;
			QObject *Plugin_;
			QByteArray VarID_;
		};
	}
}
}

Q_DECLARE_METATYPE (LeechCraft::Util::DataFilterActionInfo)

namespace LeechCraft
{
namespace Util
{
	StdDataFilterMenuCreator::StdDataFilterMenuCreator (const QVariant& dataVar, IEntityManager *em, QMenu *menu)
	: QObject (menu)
	, EntityMgr_ (em)
	{
		auto entity = MakeEntity (dataVar,
				QString (),
				static_cast<TaskParameters> (FromUserInitiated) | OnlyHandle,
				"x-leechcraft/data-filter-request");
		for (auto plugin : em->GetPossibleHandlers (entity))
		{
			auto ii = qobject_cast<IInfo*> (plugin);
			auto idf = qobject_cast<IDataFilter*> (plugin);
			if (!idf)
				continue;
			const auto& vars = idf->GetFilterVariants ();
			if (!vars.isEmpty ())
			{
				auto searchMenu = menu->addMenu (ii->GetIcon (), idf->GetFilterVerb ());
				searchMenu->menuAction ()->setIcon (ii->GetIcon ());
				for (const auto& var : vars)
				{
					auto act = searchMenu->addAction (var.Name_);
					const DataFilterActionInfo info
					{
						entity,
						plugin,
						var.ID_
					};
					act->setData (QVariant::fromValue (info));
					connect (act,
							SIGNAL (triggered ()),
							this,
							SLOT (handleDataFilterAction ()));
				}
			}
			else
			{
				auto searchAct = menu->addAction (ii->GetIcon (), idf->GetFilterVerb ());
				const DataFilterActionInfo info
				{
					entity,
					plugin,
					QByteArray ()
				};
				searchAct->setData (QVariant::fromValue (info));
				connect (searchAct,
						SIGNAL (triggered ()),
						this,
						SLOT (handleDataFilterAction ()));
			}
		}
	}

	void StdDataFilterMenuCreator::handleDataFilterAction ()
	{
		auto action = qobject_cast<QAction*> (sender ());
		const auto& data = action->data ().value<DataFilterActionInfo> ();

		auto entity = data.Entity_;
		entity.Additional_ ["DataFilter"] = data.VarID_;
		EntityMgr_->HandleEntity (entity, data.Plugin_);
	}
}
}
