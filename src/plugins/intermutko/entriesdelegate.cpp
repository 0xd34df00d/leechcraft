/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entriesdelegate.h"
#include <QSpinBox>
#include <QComboBox>
#include <QtDebug>
#include "localesmodel.h"
#include "util.h"

namespace LC
{
namespace Intermutko
{
	QWidget* EntriesDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem&, const QModelIndex& index) const
	{
		switch (static_cast<LocalesModel::Column> (index.column ()))
		{
		case LocalesModel::Column::Language:
		case LocalesModel::Column::Country:
			return new QComboBox { parent };
		case LocalesModel::Column::Quality:
		{
			const auto spinbox = new QDoubleSpinBox { parent };
			spinbox->setRange (0, 1);
			return spinbox;
		}
		case LocalesModel::Column::Code:
			return nullptr;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown column"
				<< index;
		return nullptr;
	}

	void EntriesDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
	{
		const auto& entryVar = index.data (static_cast<int> (LocalesModel::Role::LocaleEntry));
		const auto& entry = entryVar.value<LocaleEntry> ();

		switch (static_cast<LocalesModel::Column> (index.column ()))
		{
		case LocalesModel::Column::Language:
		{
			const auto combo = static_cast<QComboBox*> (editor);
			FillLanguageCombobox (combo);
			combo->setCurrentIndex (combo->findData (entry.Language_));
			break;
		}
		case LocalesModel::Column::Country:
		{
			const auto combo = static_cast<QComboBox*> (editor);
			FillCountryCombobox (combo, entry.Language_);
			combo->setCurrentIndex (combo->findData (entry.Country_));
			break;
		}
		case LocalesModel::Column::Quality:
			static_cast<QDoubleSpinBox*> (editor)->setValue (entry.Q_);
			break;
		case LocalesModel::Column::Code:
			break;
		}
	}

	void EntriesDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
	{
		switch (static_cast<LocalesModel::Column> (index.column ()))
		{
		case LocalesModel::Column::Language:
		case LocalesModel::Column::Country:
		{
			const auto combo = static_cast<QComboBox*> (editor);
			model->setData (index, combo->itemData (combo->currentIndex ()));
			break;
		}
		case LocalesModel::Column::Quality:
			model->setData (index, static_cast<QDoubleSpinBox*> (editor)->value ());
			break;
		case LocalesModel::Column::Code:
			break;
		}
	}
}
}
