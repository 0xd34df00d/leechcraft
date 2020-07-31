/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "stddatafiltermenucreator.h"
#include <QVariant>
#include <QMenu>
#include <util/sll/slotclosure.h>
#include <interfaces/iinfo.h>
#include <interfaces/idatafilter.h>
#include <interfaces/core/ientitymanager.h>
#include "util.h"

namespace LC
{
namespace Util
{
	namespace
	{
		template<typename T>
		void AddDatafilterMenuItem (const IDataFilter::FilterVariant& var, QMenu *menu, T actor)
		{
				const auto act = menu->addAction (var.Icon_, var.Name_);
				new Util::SlotClosure<Util::DeleteLaterPolicy>
				{
					[var, actor] () mutable { actor (var); },
					act,
					SIGNAL (triggered ()),
					act
				};
		}
	}

	StdDataFilterMenuCreator::StdDataFilterMenuCreator (const QVariant& dataVar, IEntityManager *em, QMenu *menu)
	: QObject (menu)
	, EntityMgr_ (em)
	{
		auto entity = MakeEntity (dataVar,
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/data-filter-request");
		for (auto plugin : em->GetPossibleHandlers (entity))
		{
			auto ii = qobject_cast<IInfo*> (plugin);
			auto idf = qobject_cast<IDataFilter*> (plugin);
			if (!idf)
				continue;

			const auto& vars = idf->GetFilterVariants (dataVar);

			if (vars.isEmpty ())
				continue;

			const auto actor = [this, entity, plugin] (const IDataFilter::FilterVariant& var) mutable
					{
						entity.Additional_ ["DataFilter"] = var.ID_;
						EntityMgr_->HandleEntity (entity, plugin);

						ChosenPlugin_ = qobject_cast<IInfo*> (plugin)->GetUniqueID ();
						ChosenVariant_ = var.ID_;
					};

			if (vars.size () == 1)
				AddDatafilterMenuItem (vars.value (0), menu, actor);
			else
			{
				auto searchMenu = menu->addMenu (ii->GetIcon (), idf->GetFilterVerb ());
				for (const auto& var : vars)
					AddDatafilterMenuItem (var, searchMenu, actor);
			}
		}
	}

	const QByteArray& StdDataFilterMenuCreator::GetChosenPlugin () const
	{
		return ChosenPlugin_;
	}

	const QByteArray& StdDataFilterMenuCreator::GetChosenVariant () const
	{
		return ChosenVariant_;
	}
}
}
