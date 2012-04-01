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

#include "restoresessiondialog.h"
#include <interfaces/iinfo.h>

namespace LeechCraft
{
namespace TabSessManager
{
	RestoreSessionDialog::RestoreSessionDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	void RestoreSessionDialog::SetPages (const QHash<QObject*, QList<RecInfo>>& pages)
	{
		Q_FOREACH (QObject *obj, pages.keys ())
		{
			IInfo *ii = qobject_cast<IInfo*> (obj);
			auto parent = new QTreeWidgetItem (QStringList (ii->GetName ()));
			parent->setIcon (0, ii->GetIcon ());
			parent->setData (0, Qt::UserRole,
					QVariant::fromValue<QObject*> (obj));
			Ui_.Tabs_->addTopLevelItem (parent);

			Q_FOREACH (const RecInfo& info, pages [obj])
			{
				const auto& name = info.Name_.isEmpty () ?
						'<' + tr ("no name") + '>' :
						info.Name_;
				auto item = new QTreeWidgetItem (QStringList (name));
				item->setIcon (0, info.Icon_);
				item->setCheckState (0, Qt::Checked);
				item->setData (0, Qt::UserRole, QVariant::fromValue (info));
				parent->addChild (item);
			}

			Ui_.Tabs_->expandItem (parent);
		}
	}

	QHash<QObject*, QList<RecInfo>> RestoreSessionDialog::GetPages () const
	{
		QHash<QObject*, QList<RecInfo>> result;

		for (int i = 0, size = Ui_.Tabs_->topLevelItemCount ();
				i < size; ++i)
		{
			auto parent = Ui_.Tabs_->topLevelItem (i);

			QList<RecInfo> infos;
			for (int j = 0, jsize = parent->childCount ();
				j < jsize; ++j)
			{
				auto child = parent->child (j);

				if (child->checkState (0) != Qt::Checked)
					continue;

				infos << child->data (0, Qt::UserRole).value<RecInfo> ();
			}

			result [parent->data (0, Qt::UserRole).value<QObject*> ()] = infos;
		}

		return result;
	}
}
}
