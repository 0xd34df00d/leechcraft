/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "searcherslist.h"
#include <QInputDialog>
#include <QMenu>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include <util/xpc/util.h>
#include "core.h"

namespace LC::SeekThru
{
	SearchersList::SearchersList (const ICoreProxy_ptr& proxy, QWidget* parent)
	: QWidget { parent }
	, Proxy_ { proxy }
	{
		Ui_.setupUi (this);
		new Util::TagsCompleter (Ui_.Tags_);
		Ui_.Tags_->AddSelector ();
		Ui_.SearchersView_->setModel (&Core::Instance ());
		connect (Ui_.SearchersView_->selectionModel (),
				SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (handleCurrentChanged (const QModelIndex&)));

		auto menu = new QMenu (Ui_.ButtonAdd_);
		Ui_.ButtonAdd_->setMenu (menu);

		menu->addAction (tr ("From searchplugins.net..."),
				this,
				[proxy]
				{
					const auto& e = Util::MakeEntity (QUrl { "http://searchplugins.net" },
							QString (), FromUserInitiated | OnlyHandle);
					proxy->GetEntityManager ()->HandleEntity (e);
				});
	}

	void SearchersList::handleCurrentChanged (const QModelIndex& current)
	{
		Ui_.ButtonRemove_->setEnabled (current.isValid ());
		Ui_.InfoBox_->setEnabled (current.isValid ());

		Current_ = current;

		QString description = current.data (Core::RoleDescription).toString ();
		if (description.isEmpty ())
			Ui_.Description_->setText (tr ("No description"));
		else
			Ui_.Description_->setText (description);

		QString longName = current.data (Core::RoleLongName).toString ();
		if (longName.isEmpty ())
			Ui_.LongName_->setText (tr ("No long name"));
		else
			Ui_.LongName_->setText (longName);

		QStringList tags = current.data (Core::RoleTags).toStringList ();
		Ui_.Tags_->setText (Proxy_->GetTagsManager ()->Join (tags));

		QString contact = current.data (Core::RoleContact).toString ();
		if (contact.isEmpty ())
			Ui_.Contact_->setText (tr ("No contacts information"));
		else
			Ui_.Contact_->setText (contact);

		QString developer = current.data (Core::RoleDeveloper).toString ();
		if (developer.isEmpty ())
			Ui_.Developer_->setText (tr ("No developer information"));
		else
			Ui_.Developer_->setText (developer);

		QString attribution = current.data (Core::RoleAttribution).toString ();
		if (attribution.isEmpty ())
			Ui_.Attribution_->setText (tr ("No attribution information"));
		else
			Ui_.Attribution_->setText (attribution);

		QString right = current.data (Core::RoleRight).toString ();
		if (right.isEmpty ())
			Ui_.Right_->setText (tr ("No right information"));
		else
			Ui_.Right_->setText (right);

		bool adult = current.data (Core::RoleAdult).toBool ();
		Ui_.Adult_->setText (adult ? tr ("Yes") : tr ("No"));

		QStringList languages = current.data (Core::RoleLanguages).toStringList ();
		Ui_.Languages_->addItems (languages);
	}

	void SearchersList::on_ButtonAdd__released ()
	{
		QString url = QInputDialog::getText (this,
				tr ("Adding a new searcher"),
				tr ("Enter the URL of the OpenSearch description"));

		if (url.isEmpty ())
			return;

		Core::Instance ().Add (url);
	}

	void SearchersList::on_ButtonRemove__released ()
	{
		Core::Instance ().Remove (Ui_.SearchersView_->selectionModel ()->currentIndex ());
	}

	void SearchersList::on_Tags__editingFinished ()
	{
		Core::Instance ().SetTags (Current_, Proxy_->GetTagsManager ()->Split (Ui_.Tags_->text ()));
	}
}
