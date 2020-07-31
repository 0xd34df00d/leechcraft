/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "restoresessiondialog.h"
#include <util/sll/qtutil.h>
#include <interfaces/iinfo.h>

namespace LC
{
namespace TabSessManager
{
	RestoreSessionDialog::RestoreSessionDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
	}

	void RestoreSessionDialog::SetTabs (const QHash<QObject*, QList<RecInfo>>& pages)
	{
		for (const auto pair : Util::Stlize (pages))
		{
			const auto obj = pair.first;
			const auto ii = qobject_cast<IInfo*> (obj);

			auto parent = new QTreeWidgetItem ({ ii->GetName () });
			parent->setIcon (0, ii->GetIcon ());
			parent->setData (0, Qt::UserRole,
					QVariant::fromValue<QObject*> (obj));
			Ui_.Tabs_->addTopLevelItem (parent);

			for (const auto& info : pair.second)
			{
				const auto& name = info.Name_.isEmpty () ?
						'<' + tr ("no name") + '>' :
						info.Name_;
				auto item = new QTreeWidgetItem ({ name });
				item->setIcon (0, info.Icon_);
				item->setCheckState (0, Qt::Checked);
				item->setData (0, Qt::UserRole, QVariant::fromValue (info));
				parent->addChild (item);
			}

			Ui_.Tabs_->expandItem (parent);
		}
	}

	QHash<QObject*, QList<RecInfo>> RestoreSessionDialog::GetTabs () const
	{
		QHash<QObject*, QList<RecInfo>> result;

		for (int i = 0, size = Ui_.Tabs_->topLevelItemCount (); i < size; ++i)
		{
			const auto parent = Ui_.Tabs_->topLevelItem (i);

			QList<RecInfo> infos;
			for (int j = 0, jsize = parent->childCount (); j < jsize; ++j)
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

	void RestoreSessionDialog::CheckAll (Qt::CheckState state)
	{
		for (int i = 0, size = Ui_.Tabs_->topLevelItemCount ();
				i < size; ++i)
		{
			auto parent = Ui_.Tabs_->topLevelItem (i);
			for (int j = 0, jsize = parent->childCount (); j < jsize; ++j)
				parent->child (j)->setCheckState (0, state);
		}
	}

	void RestoreSessionDialog::on_SelectAll__released ()
	{
		CheckAll (Qt::Checked);
	}

	void RestoreSessionDialog::on_SelectNone__released ()
	{
		CheckAll (Qt::Unchecked);
	}
}
}
