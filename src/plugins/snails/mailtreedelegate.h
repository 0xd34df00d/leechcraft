/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <QStyledItemDelegate>

class QByteArray;
class QTreeView;

namespace LC
{
namespace Snails
{
	enum class MailListMode;

	struct MessageInfo;

	using MessageLoader_f = std::function<std::optional<MessageInfo> (QByteArray)>;

	class MailTreeDelegate : public QStyledItemDelegate
	{
		const MessageLoader_f Loader_;
		QTreeView * const View_;

		MailListMode Mode_;

		int VerticalPadding_ = 2;
		bool ActionsHintsBalls_;
	public:
		MailTreeDelegate (const MessageLoader_f&, QTreeView*, QObject* = nullptr);

		void paint (QPainter*, const QStyleOptionViewItem&, const QModelIndex&) const override;
		QSize sizeHint (const QStyleOptionViewItem&, const QModelIndex&) const override;

		bool editorEvent (QEvent*, QAbstractItemModel*, const QStyleOptionViewItem&, const QModelIndex&) override;
		QWidget* createEditor (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;
		void updateEditorGeometry (QWidget*, const QStyleOptionViewItem&, const QModelIndex&) const override;

		bool eventFilter (QObject*, QEvent*) override;

		void SetMailListMode (MailListMode);
	private:
		int GetTextualHeight (const QModelIndex&, const QStyleOptionViewItem&) const;
		void DrawIcon (QPainter*, QStyleOptionViewItem&, const QModelIndex&) const;

		int DrawMessageActionIcons (QPainter*, const QStyleOptionViewItem&, const QModelIndex&, int) const;
	};
}
}
