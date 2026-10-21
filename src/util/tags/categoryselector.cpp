/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "categoryselector.h"
#include <QAction>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QVBoxLayout>

namespace LC::Util
{
	class CategorySelector::CheckableModel final : public QSortFilterProxyModel
	{
		CategorySelector& Selector_;
		QSet<QString> Selected_;
		QString Caption_;
	public:
		explicit CheckableModel (CategorySelector& selector)
		: QSortFilterProxyModel { &selector }
		, Selector_ { selector }
		{
			setSortCaseSensitivity (Qt::CaseInsensitive);
			sort (0);
		}

		QVariant headerData (int section, Qt::Orientation orientation, int role) const override
		{
			if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section)
				return {};

			return Caption_;
		}

		Qt::ItemFlags flags (const QModelIndex& index) const override
		{
			const auto flags = QSortFilterProxyModel::flags (index);
			if (!index.isValid () || index.column ())
				return flags;

			return (flags & ~Qt::ItemIsEditable) | Qt::ItemIsUserCheckable;
		}

		QVariant data (const QModelIndex& index, int role) const override
		{
			if (role != Qt::CheckStateRole)
				return QSortFilterProxyModel::data (index, role);

			if (!index.isValid () || index.column ())
				return {};

			return Selected_.contains (Name (index)) ? Qt::Checked : Qt::Unchecked;
		}

		bool setData (const QModelIndex& index, const QVariant& value, int role) override
		{
			if (role != Qt::CheckStateRole || !index.isValid () || index.column ())
				return false;

			const auto& name = Name (index);
			if (value.value<Qt::CheckState> () == Qt::Checked)
				Selected_ << name;
			else
				Selected_.remove (name);
			emit dataChanged (index, index, { Qt::CheckStateRole });

			emit Selector_.selectionEdited (Selector_.GetSelections ());
			return true;
		}

		QStringList Selections () const
		{
			QStringList result;
			for (int row = 0; row < rowCount (); ++row)
				if (const auto& name = Name (index (row, 0)); Selected_.contains (name))
					result << name;
			return result;
		}

		QModelIndexList SelectedIndexes () const
		{
			QModelIndexList result;
			for (int row = 0; row < rowCount (); ++row)
				if (const auto& idx = index (row, 0); Selected_.contains (Name (idx)))
					result << mapToSource (idx);
			return result;
		}

		void SetSelected (const QStringList& names)
		{
			Selected_ = { names.begin (), names.end () };
			NotifyAll ();
		}

		void SelectAll ()
		{
			Selected_.clear ();
			for (int row = 0; row < rowCount (); ++row)
				Selected_ << Name (index (row, 0));
			NotifyAll ();
		}

		void SelectNone ()
		{
			Selected_.clear ();
			NotifyAll ();
		}

		void SetCaption (const QString& caption)
		{
			if (caption == Caption_)
				return;

			Caption_ = caption;
			emit headerDataChanged (Qt::Horizontal, 0, 0);
		}
	private:
		QString Name (const QModelIndex& index) const
		{
			return index.siblingAtColumn (0).data ().toString ();
		}

		void NotifyAll ()
		{
			if (const auto rows = rowCount ())
				emit dataChanged (index (0, 0), index (rows - 1, 0), { Qt::CheckStateRole });
		}
	};

	CategorySelector::CategorySelector (QWidget *parent)
	: QWidget { parent }
	, Model_ { *new CheckableModel { *this } }
	, View_ { *new QTreeView { this } }
	{
		View_.setModel (&Model_);
		View_.setUniformRowHeights (true);
		View_.setRootIsDecorated (false);
		View_.setHeaderHidden (true);
		View_.setContextMenuPolicy (Qt::ActionsContextMenu);

		const auto all = new QAction { tr ("Select all"), this };
		all->setProperty ("ActionIcon", "edit-select-all");
		connect (all,
				&QAction::triggered,
				this,
				[this]
				{
					Model_.SelectAll ();
					emit selectionEdited (GetSelections ());
				});

		const auto none = new QAction { tr ("Select none"), this };
		none->setProperty ("ActionIcon", "edit-select-none");
		connect (none,
				&QAction::triggered,
				this,
				[this]
				{
					Model_.SelectNone ();
					emit selectionEdited ({});
				});

		View_.addActions ({ all, none });

		const auto layout = new QVBoxLayout { this };
		layout->setContentsMargins (0, 0, 0, 0);
		layout->addWidget (&View_);
	}

	void CategorySelector::SetModel (QAbstractItemModel& model)
	{
		Model_.setSourceModel (&model);
	}

	void CategorySelector::SetCaption (const QString& caption)
	{
		Model_.SetCaption (caption);
		View_.setHeaderHidden (caption.isEmpty ());
	}

	QStringList CategorySelector::GetSelections () const
	{
		return Model_.Selections ();
	}

	QModelIndexList CategorySelector::GetSelectedIndexes () const
	{
		return Model_.SelectedIndexes ();
	}

	void CategorySelector::SetSelections (const QStringList& names)
	{
		Model_.SetSelected (names);
	}

	void CategorySelector::SelectAll ()
	{
		Model_.SelectAll ();
	}

	void CategorySelector::SelectNone ()
	{
		Model_.SelectNone ();
	}
}
