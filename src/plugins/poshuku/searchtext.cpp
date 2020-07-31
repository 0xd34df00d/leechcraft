/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "searchtext.h"
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include "core.h"

namespace LC
{
namespace Poshuku
{
	SearchText::SearchText (const QString& text, const ICoreProxy_ptr& proxy, QWidget *parent)
	: QDialog { parent }
	, Proxy_ { proxy }
	, Text_ { text }
	{
		Ui_.setupUi (this);
		Ui_.Label_->setText (tr ("Search %1 with:").arg ("<em>" + text + "</em>"));

		for (const auto& cat : Core::Instance ().GetProxy ()->GetSearchCategories ())
			new QTreeWidgetItem (Ui_.Tree_, QStringList { cat });

		on_MarkAll__released ();

		connect (this,
				&QDialog::accepted,
				this,
				&SearchText::DoSearch);
	}

	void SearchText::DoSearch ()
	{
		QStringList selected;
		for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
			if (Ui_.Tree_->topLevelItem (i)->checkState (0) == Qt::Checked)
				selected << Ui_.Tree_->topLevelItem (i)->text (0);
		if (selected.isEmpty ())
			return;

		auto e = Util::MakeEntity (Text_,
				QString (),
				FromUserInitiated,
				"x-leechcraft/category-search-request");

		e.Additional_ ["Categories"] = selected;

		Proxy_->GetEntityManager ()->HandleEntity (e);
	}

	void SearchText::on_MarkAll__released ()
	{
		for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
			Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Checked);
	}

	void SearchText::on_UnmarkAll__released ()
	{
		for (int i = 0; i < Ui_.Tree_->topLevelItemCount (); ++i)
			Ui_.Tree_->topLevelItem (i)->setCheckState (0, Qt::Unchecked);
	}
}
}
