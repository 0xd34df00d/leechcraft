/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <util/models/itemsmodel.h>
#include "types.h"
#include "ui_importopmldialog.h"

namespace LC::Aggregator
{
	struct OPMLItem;

	class ImportOPMLDialog : public QDialog
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Aggregator::ImportOPMLDialog)

		Ui::ImportOPMLDialog Ui_;

		struct Item : OPMLItem
		{
			explicit Item (const OPMLItem& item)
			: OPMLItem { item }
			{
			}

			bool IsChecked_ = true;
		};
		using ItemsModel_t = Util::ItemsModel<Item, Util::ItemsCheckable<&Item::IsChecked_>, Util::ItemsEditable>;
		ItemsModel_t Model_;

		QHash<QStringView, QLabel*> Fields_;
	public:
		struct Browse {};
		struct FilePath { QString File_; };
		struct OpmlContents { QByteArray Contents_; QString SourceName_; };

		using Source = std::variant<Browse, FilePath, OpmlContents>;

		explicit ImportOPMLDialog (const Source& = Browse {}, QWidget* = nullptr);
		~ImportOPMLDialog () override;

		QString GetTags () const;
		QList<OPMLItem> GetSelectedItems () const;
	private:
		void HandleFilePathEdited (const QString&);
		void BrowseFile ();

		void HandleDevice (QIODevice&, const QString&);
		void Reset ();
	};
}
