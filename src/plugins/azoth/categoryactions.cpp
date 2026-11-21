/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "categoryactions.h"
#include <QMenu>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/asyncdialogs.h>
#include <util/sll/prelude.h>
#include <util/sll/trackingpointer.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "components/dialogs/groupsenddialog.h"
#include "components/dialogs/groupremovedialog.h"
#include "components/util/gui.h"

namespace LC::Azoth::Actions
{
	namespace
	{
		Util::Task<void> RenameCategory (Util::TrackingPointer<IAccount> account, QString name)
		{
			const auto& newName = co_await Util::Dialogs::GetText ({
						.Parent_ = GetDialogParent (),
						.Title_ = CategoryActions::tr ("Rename group"),
						.Label_ = CategoryActions::tr ("Enter new group name for %1:").arg (name),
						.Text_ = name,
					});
			if (!newName || newName->isEmpty () || *newName == name)
				co_return;

			for (const auto entry : account->GetCLEntries ())
			{
				auto groups = entry->Groups ();
				if (groups.removeAll (name))
				{
					if (!groups.contains (*newName))
						groups << *newName;
					entry->SetGroups (groups);
				}
			}
		}
	}

	void PopulateMenu (QMenu *menu, const CategoryInfo& cat)
	{
		const auto itm = GetProxyHolder ()->GetIconThemeManager ();

		menu->addAction (itm->GetIcon ("edit-rename"),
				CategoryActions::tr ("Rename group..."),
				menu,
				[account = cat.Account_, name = cat.Name_] { RenameCategory (account, name); });

		menu->addAction (itm->GetIcon ("mail-send"),
				CategoryActions::tr ("Send message..."),
				menu,
				[entries = cat.Entries_]
				{
					auto dlg = new GroupSendDialog { entries, GetDialogParent () };
					dlg->setAttribute (Qt::WA_DeleteOnClose, true);
					dlg->show ();
				});

		if (cat.UnreadCount_)
			menu->addAction (itm->GetIcon ("mail-mark-read"),
					CategoryActions::tr ("Mark all messages as read"),
					menu,
					[entries = cat.Entries_]
					{
						for (const auto entry : entries)
							entry->MarkMsgsRead ();
					});

		const auto removableEntries = Util::Filter (cat.Entries_,
					[] (ICLEntry *entry)
					{
						return (entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) == ICLEntry::FPermanentEntry;
					});
		if (!removableEntries.isEmpty ())
			menu->addAction (itm->GetIcon ("list-remove"),
					CategoryActions::tr ("Remove group's participants..."),
					menu,
					[removableEntries]
					{
						auto dlg = new GroupRemoveDialog { removableEntries, GetDialogParent () };
						dlg->setAttribute (Qt::WA_DeleteOnClose, true);
						dlg->show ();
					});
	}
}
