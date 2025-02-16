/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "localesmodel.h"
#include <QtDebug>
#include <util/sll/util.h>
#include "util.h"

namespace LC::Intermutko
{
	int LocalesModel::columnCount (const QModelIndex& parent) const
	{
		return parent.isValid () ?
				0 :
				Headers_.size ();
	}

	int LocalesModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ?
				0 :
				Locales_.size ();
	}

	QModelIndex LocalesModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (parent.isValid ())
			return {};

		return createIndex (row, column);
	}

	QModelIndex LocalesModel::parent (const QModelIndex&) const
	{
		return {};
	}

	QVariant LocalesModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Vertical)
			return {};

		if (role != Qt::DisplayRole)
			return {};

		return Headers_.value (section);
	}

	QVariant LocalesModel::data (const QModelIndex& index, int role) const
	{
		const auto& entry = Locales_.value (index.row ());
		if (role == static_cast<int> (Role::LocaleEntry))
			return QVariant::fromValue (entry);

		if (role != Qt::DisplayRole && role != Qt::EditRole)
			return {};

		switch (static_cast<Column> (index.column ()))
		{
		case Column::Language:
			return entry.Locale_.nativeLanguageName ();
		case Column::Country:
			return GetCountryName (entry.Locale_);
		case Column::Quality:
			return entry.Q_;
		case Column::Code:
			return GetDisplayCode (entry.Locale_);
		}
		return {};
	}

	Qt::ItemFlags LocalesModel::flags (const QModelIndex& index) const
	{
		Qt::ItemFlags result = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		if (index.column () != static_cast<int> (Column::Code))
			result |= Qt::ItemIsEditable;
		return result;
	}

	bool LocalesModel::setData (const QModelIndex& idx, const QVariant& value, int)
	{
		if (!idx.isValid ())
			return false;

		auto& entry = Locales_ [idx.row ()];
		auto& locale = entry.Locale_;
		switch (static_cast<Column> (idx.column ()))
		{
		case Column::Language:
			locale = QLocale { static_cast<QLocale::Language> (value.toInt ()), locale.script (), locale.territory () };
			break;
		case Column::Country:
			locale = QLocale { locale.language (), locale.script (), static_cast<QLocale::Country> (value.toInt ()) };
			break;
		case Column::Quality:
			entry.Q_ = value.toDouble ();
			break;
		default:
			return false;
		}

		emit dataChanged (index (idx.row (), 0), index (idx.row (), columnCount () - 1));

		return true;
	}

	const QList<LocaleEntry>& LocalesModel::GetEntries () const
	{
		return Locales_;
	}

	void LocalesModel::AddLocaleEntry (const LocaleEntry& entry)
	{
		beginInsertRows ({}, Locales_.size (), Locales_.size ());
		Locales_ << entry;
		endInsertRows ();
	}

	void LocalesModel::SetLocales (const QList<LocaleEntry>& entries)
	{
		if (const auto rc = Locales_.size ())
		{
			beginRemoveRows ({}, 0, rc - 1);
			Locales_.clear ();
			endRemoveRows ();
		}

		if (const auto rc = entries.size ())
		{
			beginInsertRows ({}, 0, rc - 1);
			Locales_ = entries;
			endInsertRows ();
		}
	}

	void LocalesModel::Remove (const QModelIndex& index)
	{
		if (!index.isValid ())
			return;

		const auto r = index.row ();
		beginRemoveRows ({}, r, r);
		Locales_.removeAt (r);
		endRemoveRows ();
	}

	void LocalesModel::MoveUp (const QModelIndex& index)
	{
		if (!index.isValid () || !index.row ())
			return;

		const auto r = index.row ();

		beginRemoveRows ({}, r - 1, r - 1);
		const auto& preRow = Locales_.takeAt (r - 1);
		endRemoveRows ();

		beginInsertRows ({}, r, r);
		Locales_.insert (r, preRow);
		endInsertRows ();
	}

	void LocalesModel::MoveDown (const QModelIndex& index)
	{
		if (!index.isValid () || index.row () == Locales_.size () - 1)
			return;

		const auto r = index.row ();
		beginRemoveRows ({}, r + 1, r + 1);
		const auto& postRow = Locales_.takeAt (r + 1);
		endRemoveRows ();

		beginInsertRows ({}, r, r);
		Locales_.insert (r, postRow);
		endInsertRows ();
	}
}
