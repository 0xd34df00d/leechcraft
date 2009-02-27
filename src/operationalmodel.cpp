#include "operationalmodel.h"
#include <interfaces/ifinder.h>

using namespace LeechCraft;
using namespace LeechCraft::Util;

OperationalModel::OperationalModel (QObject *parent)
: MergeModel (QStringList (tr ("Entity"))
			<< tr ("Category")
			<< tr ("Information"), parent)
{
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
		QByteArray hash = model->index (row, 0).data (IFindProxy::RoleHash).toByteArray ();
		size_t sameModels = 0;

		for (models_t::const_iterator i = Models_.begin (),
				end = Models_.end (); i != end; ++i)
		{
			if (*i == model)
				continue;

			bool found = false;
			for (int j = 0; j < (*i)->rowCount (); ++j)
				if ((*i)->index (j, 0).data (IFindProxy::RoleHash).toByteArray () == hash)
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

