/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "entriesdelegate.h"
#include <QComboBox>
#include <QStringListModel>
#include <util/tags/tagslineedit.h>
#include <util/tags/tagscompleter.h>
#include <interfaces/core/itagsmanager.h>
#include "entriesmodel.h"
#include "core.h"
#include "accountsmanager.h"
#include "currenciesmanager.h"

namespace LC
{
namespace Poleemery
{
	QWidget* EntriesDelegate::createEditor (QWidget *parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case EntriesModel::Columns::Account:
		case EntriesModel::Columns::EntryCurrency:
			return new QComboBox (parent);
		case EntriesModel::Columns::Categories:
		{
			auto result = new Util::TagsLineEdit (parent);
			auto completer = new Util::TagsCompleter (result);

			const auto& cats = Core::Instance ().GetOpsManager ()->GetKnownCategories ().toList ();
			completer->OverrideModel (new QStringListModel (cats, completer));

			result->AddSelector ();

			return result;
		}
		default:
			return QStyledItemDelegate::createEditor (parent, option, index);
		}
	}

	void EntriesDelegate::setEditorData (QWidget *editor, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case EntriesModel::Columns::Account:
		{
			auto box = qobject_cast<QComboBox*> (editor);
			const auto currentAccId = index.data (Qt::EditRole).toInt ();
			int toFocus = -1;
			for (const auto& acc : Core::Instance ().GetAccsManager ()->GetAccounts ())
			{
				if (acc.ID_ == currentAccId)
					toFocus = box->count ();
				box->addItem (acc.Name_, static_cast<int> (acc.ID_));
			}
			box->setCurrentIndex (toFocus);
			break;
		}
		case EntriesModel::Columns::EntryCurrency:
		{
			auto box = qobject_cast<QComboBox*> (editor);
			const auto currentCurrency = index.data (Qt::EditRole).toString ();

			const auto& enabledCurrencies = Core::Instance ()
					.GetCurrenciesManager ()->GetEnabledCurrencies ();
			box->addItems (enabledCurrencies);

			const auto idx = enabledCurrencies.indexOf (currentCurrency);
			box->setCurrentIndex (idx);

			break;
		}
		case EntriesModel::Columns::Categories:
		{
			auto edit = qobject_cast<Util::TagsLineEdit*> (editor);
			edit->setTags (index.data (Qt::EditRole).toStringList ());
			break;
		}
		default:
			QStyledItemDelegate::setEditorData (editor, index);
			break;
		}
	}

	void EntriesDelegate::setModelData (QWidget *editor, QAbstractItemModel *model, const QModelIndex& index) const
	{
		switch (index.column ())
		{
		case EntriesModel::Columns::Account:
		{
			auto box = qobject_cast<QComboBox*> (editor);
			model->setData (index, box->itemData (box->currentIndex ()));
			break;
		}
		case EntriesModel::Columns::EntryCurrency:
		{
			auto box = qobject_cast<QComboBox*> (editor);
			model->setData (index, box->currentText ());
			break;
		}
		case EntriesModel::Columns::Categories:
		{
			auto box = qobject_cast<Util::TagsLineEdit*> (editor);
			auto itm = Core::Instance ().GetCoreProxy ()->GetTagsManager ();
			model->setData (index, itm->Split (box->text ()));
			break;
		}
		default:
			QStyledItemDelegate::setModelData (editor, model, index);
			break;
		}
	}
}
}
