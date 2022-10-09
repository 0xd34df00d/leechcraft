/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "categoryactions.h"
#include <QMenu>
#include <QLineEdit>
#include <QInputDialog>
#include <util/sll/prelude.h>
#include "interfaces/azoth/iclentry.h"
#include "groupsenddialog.h"
#include "groupremovedialog.h"
#include "util.h"

namespace LC::Azoth::Actions
{
	namespace
	{
		void RenameCategory (const CategoryInfo& cat)
		{
			const auto& newName = QInputDialog::getText (GetDialogParent (),
					CategoryActions::tr ("Rename group"),
					CategoryActions::tr ("Enter new group name for %1:")
						.arg (cat.Name_),
					QLineEdit::Normal,
					cat.Name_);
			if (newName.isEmpty () || newName == cat.Name_)
				return;

			for (const auto entry : cat.Entries_)
			{
				auto groups = entry->Groups ();
				groups.removeAll (cat.Name_);
				groups << newName;
				entry->SetGroups (groups);
			}
		}
	}

	void PopulateMenu (QMenu *menu, const CategoryInfo& cat)
	{
		menu->addAction (CategoryActions::tr ("Rename group..."),
				[=] { RenameCategory (cat); });

		menu->addAction (CategoryActions::tr ("Send message..."),
				[=]
				{
					auto dlg = new GroupSendDialog { cat.Entries_, GetDialogParent () };
					dlg->setAttribute (Qt::WA_DeleteOnClose, true);
					dlg->show ();
				});

		if (cat.UnreadCount_)
			menu->addAction (CategoryActions::tr ("Mark all messages as read"),
					[=]
					{
						for (const auto entry : cat.Entries_)
							entry->MarkMsgsRead ();
					});

		const auto removableEntries = Util::Filter (cat.Entries_,
					[] (ICLEntry *entry)
					{
						return (entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) == ICLEntry::FPermanentEntry;
					});
		if (!removableEntries.isEmpty ())
			menu->addAction (CategoryActions::tr ("Remove group's participants"),
					[removableEntries]
					{
						auto dlg = new GroupRemoveDialog { removableEntries, GetDialogParent () };
						dlg->setAttribute (Qt::WA_DeleteOnClose, true);
						dlg->show ();
					});
	}
}
