/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "operationalmodel.h"
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			OperationalModel::OperationalModel (QObject *parent)
			: MergeModel (QStringList (tr ("Entity"))
						<< tr ("Category")
						<< tr ("Information"), parent)
			{
				setProperty ("__LeechCraft_own_core_model", true);
			}

			void OperationalModel::SetOperation (Operation op)
			{
				Op_ = op;
			}

			bool OperationalModel::AcceptsRow (QAbstractItemModel *model,
					int row) const
			{
				if (Op_ == OpAnd)
				{
					QByteArray hash = model->index (row, 0).data (LeechCraft::RoleHash).toByteArray ();
					size_t sameModels = 0;

					for (models_t::const_iterator i = Models_.begin (),
							end = Models_.end (); i != end; ++i)
					{
						if (*i == model)
							continue;

						bool found = false;
						for (int j = 0; j < (*i)->rowCount (); ++j)
							if ((*i)->index (j, 0).data (LeechCraft::RoleHash).toByteArray () == hash)
							{
								++sameModels;
								found = true;
								break;
							}
						if (!found)
							break;
					}
					return sameModels == Models_.size () - 1;
				}
				else
					return true;
			}
		};
	};
};

