#include "operationalmodel.h"
#include <interfaces/ifinder.h>
#include <interfaces/structures.h>

using namespace LeechCraft;
using namespace LeechCraft::Util;

LeechCraft::OperationalModel::OperationalModel (QObject *parent)
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

