/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "categoryselector.h"
#include <algorithm>
#include <QStringList>
#include <QCheckBox>
#include <QVariant>
#include <QVBoxLayout>
#include <QMoveEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QStringListModel>
#include <QAction>
#include <QtDebug>
#include "ui_categoryselector.h"
#include "util.h"

namespace LC::Util
{
	class SelectorTagsModel : public QStringListModel
	{
		CategorySelector& Selector_;
	public:
		explicit SelectorTagsModel (CategorySelector& selector)
		: QStringListModel { &selector }
		, Selector_ { selector }
		{
		}

		Qt::ItemFlags flags (const QModelIndex& index) const override
		{
			return QStringListModel::flags (index) & ~Qt::ItemFlag::ItemIsEditable;
		}

		bool setData (const QModelIndex& index, const QVariant& value, int role) override
		{
			const auto result = QStringListModel::setData (index, value, role);
			if (role == Qt::CheckStateRole)
				Selector_.NotifyTagsSelection ();
			return result;
		}
	};

	CategorySelector::CategorySelector (QWidget *parent)
	: QDialog (parent)
	, Ui_ { new Ui::CategorySelector }
	, Model_ { *new SelectorTagsModel { *this } }
	, Separator_ { GetDefaultTagsSeparator () }
	{
		setWindowTitle (tr ("Tags selector"));
		setWindowFlags (Qt::Dialog | Qt::WindowStaysOnTopHint);

		Ui_->setupUi (this);
		Ui_->Tree_->setModel (&Model_);

		const auto& avail = QApplication::desktop ()->availableGeometry (this);
		setMinimumHeight (avail.height () / 3 * 2);

		const auto all = new QAction (tr ("Select all"), this);
		connect (all,
				&QAction::triggered,
				this,
				&CategorySelector::SelectAll);

		const auto none = new QAction (tr ("Select none"), this);
		connect (none,
				&QAction::triggered,
				this,
				&CategorySelector::SelectNone);

		Ui_->Tree_->addAction (all);
		Ui_->Tree_->addAction (none);

		Ui_->Tree_->setContextMenuPolicy (Qt::ActionsContextMenu);

		SetButtonsMode (parent ? ButtonsMode::NoButtons : ButtonsMode::Close);
	}

	void CategorySelector::SetCaption (const QString& caption)
	{
		Model_.setHeaderData (0, Qt::Horizontal, caption, Qt::DisplayRole);
		Caption_ = caption;
	}

	void CategorySelector::SetPossibleSelections (QStringList tags, bool sort)
	{
		auto guard = DisableNotifications ();

		if (sort)
			tags.sort ();
		Model_.setStringList (tags);
	}

	QStringList CategorySelector::GetSelections () const
	{
		const auto& allTags = Model_.stringList ();
		const auto& selectedIdxes = GetSelectedIndexes ();
		QStringList selected;
		selected.reserve (selectedIdxes.size ());
		for (const auto idx : selectedIdxes)
			selected << allTags [idx];
		return selected;
	}

	QList<int> CategorySelector::GetSelectedIndexes () const
	{
		QList<int> result;

		const auto& rowCount = Model_.stringList ().size ();
		for (int i = 0; i < rowCount; ++i)
		{
			const auto state = Model_.index (i).data (Qt::CheckStateRole).value<Qt::CheckState> ();
			if (state == Qt::Checked)
				result << i;
		}

		return result;
	}

	void CategorySelector::SetSelections (const QStringList& tags)
	{
		auto guard = DisableNotifications (false);

		const auto& allTags = Model_.stringList ();
		const auto rowCount = allTags.size ();
		for (int i = 0; i < rowCount; ++i)
		{
			const auto state = tags.contains (allTags [i]) ?
					Qt::Checked :
					Qt::Unchecked;
			Model_.setData (Model_.index (i), Qt::CheckStateRole, state);
		}
	}

	QString CategorySelector::GetSeparator () const
	{
		return Separator_;
	}

	void CategorySelector::SetSeparator (const QString& sep)
	{
		Separator_ = sep;
	}

	void CategorySelector::SetButtonsMode (ButtonsMode mode)
	{
		switch (mode)
		{
		case ButtonsMode::NoButtons:
			Ui_->ButtonsBox_->setVisible (false);
			break;
		case ButtonsMode::Close:
			Ui_->ButtonsBox_->setStandardButtons (QDialogButtonBox::Close);
			Ui_->ButtonsBox_->setVisible (true);
			break;
		case ButtonsMode::AcceptReject:
			Ui_->ButtonsBox_->setStandardButtons (QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
			Ui_->ButtonsBox_->setVisible (true);
			break;
		}
	}

	void CategorySelector::moveEvent (QMoveEvent *e)
	{
		QWidget::moveEvent (e);
		QPoint pos = e->pos ();
		QRect avail = QApplication::desktop ()->availableGeometry (this);
		int dx = 0, dy = 0;
		if (pos.x () + width () > avail.width ())
			dx = width () + pos.x () - avail.width ();
		if (pos.y () + height () > avail.height () &&
				height () < avail.height ())
			dy = height () + pos.y () - avail.height ();

		if (dx || dy)
			move (pos - QPoint (dx, dy));
	}

	void CategorySelector::SelectAll ()
	{
		auto guard = DisableNotifications ();

		const auto rowCount = Model_.stringList ().size ();
		for (int i = 0; i < rowCount; ++i)
			Model_.setData (Model_.index (i), Qt::CheckStateRole, Qt::Checked);
	}

	void CategorySelector::SelectNone ()
	{
		auto guard = DisableNotifications ();

		const auto rowCount = Model_.stringList ().size ();
		for (int i = 0; i < rowCount; ++i)
			Model_.setData (Model_.index (i), Qt::CheckStateRole, Qt::Unchecked);
	}

	void CategorySelector::SetSelectionsFromString (const QString& text)
	{
		auto guard = DisableNotifications (false);
		SetSelections (text.split (Separator_, Qt::SkipEmptyParts));
	}

	void CategorySelector::NotifyTagsSelection ()
	{
		if (NotificationsEnabled_)
			emit tagsSelectionChanged (GetSelections ());
	}

	DefaultScopeGuard CategorySelector::DisableNotifications (bool reemit)
	{
		auto prevValue = NotificationsEnabled_;
		NotificationsEnabled_ = false;
		return MakeScopeGuard ([this, prevValue, reemit]
				{
					NotificationsEnabled_ = prevValue;
					if (reemit)
						NotifyTagsSelection ();
				});
	}
}
