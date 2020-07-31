/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include "localeentry.h"

namespace LC
{
namespace Intermutko
{
	class LocalesModel : public QAbstractItemModel
	{
		Q_OBJECT

		QList<LocaleEntry> Locales_;

		const QStringList Headers_
		{
			tr ("Language"),
			tr ("Country"),
			tr ("Quality"),
			tr ("Code")
		};
	public:
		enum class Column
		{
			Language,
			Country,
			Quality,
			Code
		};

		enum class Role
		{
			LocaleEntry = Qt::UserRole + 1
		};

		using QAbstractItemModel::QAbstractItemModel;

		int columnCount (const QModelIndex& = {}) const override;
		int rowCount (const QModelIndex& = {}) const override;
		QModelIndex index (int, int, const QModelIndex& = {}) const override;
		QModelIndex parent (const QModelIndex&) const override;
		QVariant headerData (int, Qt::Orientation, int) const override;
		QVariant data (const QModelIndex&, int) const override;
		Qt::ItemFlags flags (const QModelIndex& index) const override;
		bool setData (const QModelIndex&, const QVariant&, int) override;

		const QList<LocaleEntry>& GetEntries () const;
		void AddLocaleEntry (const LocaleEntry&);
		void SetLocales (const QList<LocaleEntry>&);
		void Remove (const QModelIndex&);

		void MoveUp (const QModelIndex&);
		void MoveDown (const QModelIndex&);
	};
}
}
