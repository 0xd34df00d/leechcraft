/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mailtreedelegate.h"
#include <QPainter>
#include <QMouseEvent>
#include <QToolBar>
#include <QTreeView>
#include <QProxyStyle>
#include <QMenu>
#include <QTimer>
#include <QtDebug>
#include <util/sll/slotclosure.h>
#include "common.h"
#include "mailtab.h"
#include "mailmodel.h"
#include "messagelistactioninfo.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Snails
{
	MailTreeDelegate::MailTreeDelegate (const MessageLoader_f& loader,
			QTreeView *view, QObject *parent)
	: QStyledItemDelegate { parent }
	, Loader_ { loader }
	, View_ { view }
	, Mode_ { MailListMode::Normal }
	{
		XmlSettingsManager::Instance ().RegisterObject ("MessageActionsHintsStyle", this,
				[this] (const QVariant& var) { ActionsHintsBalls_ = var.toString () == "Ball"; });
		XmlSettingsManager::Instance ().RegisterObject ("MessageListPadding", this,
				[this] (const QVariant& var)
				{
					VerticalPadding_ = var.toInt ();
					View_->doItemsLayout ();
				});
	}

	const int HorizontalPadding = 2;

	namespace
	{
		QString GetString (const QModelIndex& index, MailModel::Column column)
		{
			return index.sibling (index.row (), static_cast<int> (column)).data ().toString ();
		}

		QPair<QFont, QFontMetrics> GetSubjectFont (const QModelIndex& index, QFont font)
		{
			if (!index.data (MailModel::MailRole::IsRead).toBool ())
				font.setBold (true);

			return { font, QFontMetrics { font } };
		}

		int GetActionsBarWidth (const QModelIndex& index,
				QStyle *style, const QStyleOptionViewItem& option, int subjHeight)
		{
			const auto& acts = index.data (MailModel::MailRole::MessageActions)
					.value<QList<MessageListActionInfo>> ();
			if (acts.isEmpty ())
				return 0;

			subjHeight += 5; // seems like a common extra size for QToolButtons that's impossible to figure out otherwise

			const auto spacing = style->pixelMetric (QStyle::PM_ToolBarItemSpacing, &option);

			return acts.size () * subjHeight +
					(acts.size () - 1) * spacing +
					2 * HorizontalPadding;
		}

		void DrawCheckbox (QPainter *painter, QStyle *style,
				QStyleOptionViewItem& option, const QModelIndex& index)
		{
			QStyleOptionButton checkBoxOpt;
			static_cast<QStyleOption&> (checkBoxOpt) = option;
			switch (index.data (Qt::CheckStateRole).toInt ())
			{
			case Qt::Checked:
				checkBoxOpt.state |= QStyle::State_On;
				break;
			case Qt::Unchecked:
				checkBoxOpt.state |= QStyle::State_Off;
				break;
			case Qt::PartiallyChecked:
				checkBoxOpt.state |= QStyle::State_NoChange;
				break;
			}
			style->drawControl (QStyle::CE_CheckBox, &checkBoxOpt, painter);

			const auto checkboxWidth = style->pixelMetric (QStyle::PM_IndicatorWidth) +
					style->pixelMetric (QStyle::PM_CheckBoxLabelSpacing);
			option.rect.setLeft (option.rect.left () + checkboxWidth);
		}

		QStyle* GetStyle (const QStyleOptionViewItem& option)
		{
			return option.widget ?
					option.widget->style () :
					QApplication::style ();
		}
	}

	void MailTreeDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& stockItem, const QModelIndex& index) const
	{
		auto option = stockItem;

		const auto style = GetStyle (option);

		painter->save ();

		style->drawPrimitive (QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

		if (option.state & QStyle::State_Selected)
			painter->setPen (option.palette.color (QPalette::HighlightedText));

		if (Mode_ == MailListMode::MultiSelect)
			DrawCheckbox (painter, style, option, index);

		DrawIcon (painter, option, index);

		const auto& subject = GetString (index, MailModel::Column::Subject);

		const auto& subjFontInfo = GetSubjectFont (index, option.font);
		const auto subjHeight = subjFontInfo.second.height ();
		auto y = option.rect.top () + subjHeight;

		const auto actionsWidth = View_->isPersistentEditorOpen (index) ?
				GetActionsBarWidth (index, style, option, subjHeight) :
				0;

		const auto actionsHintWidth = DrawMessageActionIcons (painter, option, index, subjHeight);

		painter->setFont (subjFontInfo.first);
		painter->drawText (option.rect.left (),
				y,
				subjFontInfo.second.elidedText (subject, Qt::ElideRight,
						option.rect.width () - actionsHintWidth - actionsWidth));

		const QFontMetrics fontFM { option.font };

		auto stringHeight = [&fontFM] (const QString&)
		{
			return fontFM.height ();
		};

		auto from = GetString (index, MailModel::Column::From);
		if (const auto childrenCount = index.data (MailModel::MailRole::TotalChildrenCount).toInt ())
		{
			from += " (";
			if (const auto unread = index.data (MailModel::MailRole::UnreadChildrenCount).toInt ())
				from += QString::number (unread) + "/";
			from += QString::number (childrenCount) + ")";
		}
		const auto& date = GetString (index, MailModel::Column::Date);

		y += std::max (stringHeight (from), stringHeight (date));

		painter->setFont (option.font);

		const auto dateWidth = fontFM.boundingRect (date).width ();
		painter->drawText (option.rect.right () - dateWidth,
				y,
				date);

		painter->drawText (option.rect.left (),
				y,
				fontFM.elidedText (from, Qt::ElideRight, option.rect.width () - dateWidth - 5 * HorizontalPadding));

		painter->restore ();
	}

	QSize MailTreeDelegate::sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		return { View_->viewport ()->width (), GetTextualHeight (index, option) + 2 * VerticalPadding_ };
	}

	bool MailTreeDelegate::editorEvent (QEvent *event, QAbstractItemModel *model,
			const QStyleOptionViewItem& option, const QModelIndex& index)
	{
		if (Mode_ != MailListMode::MultiSelect)
			return QStyledItemDelegate::editorEvent (event, model, option, index);

		switch (event->type ())
		{
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::MouseMove:
			break;
		default:
			return QStyledItemDelegate::editorEvent (event, model, option, index);
		}

		const auto mouseEvent = static_cast<QMouseEvent*> (event);
		const auto xPos = mouseEvent->pos ().x ();

		const auto style = GetStyle (option);
		const auto checkBoxWidth = style->pixelMetric (QStyle::PM_IndicatorWidth);
		const auto left = option.rect.left ();
		if (xPos < left || xPos > left + checkBoxWidth)
			return QStyledItemDelegate::editorEvent (event, model, option, index);

		if (event->type () == QEvent::MouseButtonRelease)
		{
			const auto current = index.data (Qt::CheckStateRole).toInt ();
			const auto desired = current == Qt::Checked ?
					Qt::Unchecked :
					Qt::Checked;
			model->setData (index, desired, Qt::CheckStateRole);

			if (mouseEvent->button () == Qt::RightButton)
			{
				QList<QModelIndex> children { index };

				while (!children.isEmpty ())
				{
					const auto& nextIndex = children.takeFirst ();
					model->setData (nextIndex, desired, Qt::CheckStateRole);

					for (int i = 0, rc = model->rowCount (nextIndex); i < rc; ++i)
						children << model->index (i, 0, nextIndex);
				}
			}
		}

		return true;
	}

	namespace
	{
		class NullMarginsStyle : public QProxyStyle
		{
		public:
			using QProxyStyle::QProxyStyle;

			int pixelMetric (PixelMetric metric, const QStyleOption *option, const QWidget *widget) const override
			{
				switch (metric)
				{
				case QStyle::PM_ToolBarItemMargin:
				case QStyle::PM_ToolBarFrameWidth:
				case QStyle::PM_ToolBarHandleExtent:
				case QStyle::PM_ToolBarExtensionExtent:
				case QStyle::PM_ToolBarItemSpacing:
					return 0;
				default:
					return QProxyStyle::pixelMetric (metric, option, widget);
				}
			}
		};

		template<typename Loader, typename ContainerT>
		void BuildAction (const Loader& loader, ContainerT *container, const MessageListActionInfo& actInfo)
		{
			const auto action = container->addAction (actInfo.Icon_, actInfo.Name_);
			action->setToolTip (actInfo.Description_);

			if (actInfo.Children_.isEmpty ())
				new Util::SlotClosure<Util::NoDeletePolicy>
				{
					[loader, handler = actInfo.Handler_]
					{
						if (auto item = loader ())
							handler (*item);
						else
							qWarning () << Q_FUNC_INFO
									<< "unable to load message";
					},
					action,
					SIGNAL (triggered ()),
					action
				};
			else
			{
				auto menu = new QMenu;
				menu->setIcon (actInfo.Icon_);
				menu->setTitle (actInfo.Name_);
				action->setMenu (menu);

				for (const auto& child : actInfo.Children_)
					BuildAction (loader, menu, child);
			}
		}
	}

	QWidget* MailTreeDelegate::createEditor (QWidget *parent,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const auto& actionsVar = index.data (MailModel::MailRole::MessageActions);
		if (actionsVar.isNull ())
			return nullptr;

		const auto& actionInfos = actionsVar.value<QList<MessageListActionInfo>> ();
		if (actionInfos.isEmpty ())
			return nullptr;

		const auto& id = index.data (MailModel::MailRole::ID).toByteArray ();

		const auto container = new QToolBar { parent };
		container->setStyleSheet ("QToolButton { margin: 0; padding: 0; border-width: 1px; }");

		static auto style = new NullMarginsStyle;
		container->setStyle (style);

		for (const auto& actInfo : actionInfos)
			BuildAction (std::bind (Loader_, id), container, actInfo);

		QTimer::singleShot (0, this, [=] { updateEditorGeometry (container, option, index); });

		return container;
	}

	void MailTreeDelegate::updateEditorGeometry (QWidget *editor,
			const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		auto height = GetSubjectFont (index, option.font).second.height ();

		qobject_cast<QToolBar*> (editor)->setIconSize ({ height, height });

		editor->setMaximumSize (option.rect.size ());
		editor->move (option.rect.topRight () - QPoint { editor->width (), 0 });
	}

	bool MailTreeDelegate::eventFilter (QObject *object, QEvent *event)
	{
		QSignalBlocker blocker { this };
		return QStyledItemDelegate::eventFilter (object, event);
	}

	void MailTreeDelegate::SetMailListMode (MailListMode mode)
	{
		if (Mode_ == mode)
			return;

		Mode_ = mode;

		auto idx = View_->indexAt (View_->rect ().topLeft ());
		while (idx.isValid ())
		{
			View_->update (idx);
			idx = View_->indexBelow (idx);
		}
	}

	int MailTreeDelegate::GetTextualHeight (const QModelIndex& index, const QStyleOptionViewItem& option) const
	{
		const auto& subjFontInfo = GetSubjectFont (index, option.font);
		const QFontMetrics plainFM { option.font };
		return subjFontInfo.second.height () + plainFM.height ();
	}

	void MailTreeDelegate::DrawIcon (QPainter *painter, QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		const auto height = GetTextualHeight (index, option);

		const auto& px = index.data (Qt::DecorationRole).value<QIcon> ()
				.pixmap (height, height);

		auto topLeft = option.rect.topLeft ();
		topLeft.ry () += (height - px.height ()) / 2;
		painter->drawPixmap (topLeft, px);

		option.rect.adjust (px.width () + HorizontalPadding, 0, 0, 0);
	}

	int MailTreeDelegate::DrawMessageActionIcons (QPainter *painter,
			const QStyleOptionViewItem& option, const QModelIndex& index, int height) const
	{
		if (Mode_ != MailListMode::Normal)
			return 0;

		if (option.state & QStyle::State_MouseOver)
			return 0;

		const auto& actionsVar = index.data (MailModel::MailRole::MessageActions);
		if (actionsVar.isNull ())
			return 0;

		auto actionInfos = actionsVar.value<QList<MessageListActionInfo>> ();
		if (actionInfos.isEmpty ())
			return 0;

		std::reverse (actionInfos.begin (), actionInfos.end ());

		if (ActionsHintsBalls_)
			height -= VerticalPadding_ * 2;

		painter->save ();
		painter->setRenderHint (QPainter::Antialiasing);

		painter->setPen (Qt::NoPen);

		auto rect = option.rect;
		rect.setLeft (rect.right () - height - VerticalPadding_);
		rect.setSize ({ height, height });
		rect.moveTop (rect.top () + VerticalPadding_);
		for (const auto& item : actionInfos)
		{
			if (item.Flags_ & MessageListActionFlag::AlwaysPresent)
				continue;

			if (ActionsHintsBalls_)
			{
				QRadialGradient gradient;
				gradient.setCoordinateMode (QGradient::ObjectBoundingMode);
				gradient.setFocalPoint ({ 0.3, 0.3 });
				gradient.setCenter ({ 0.5, 0.5 });
				gradient.setRadius (0.5);
				gradient.setColorAt (0, item.ReprColor_.lighter (200));
				gradient.setColorAt (1, item.ReprColor_.darker (120));

				painter->setBrush (gradient);
				painter->drawEllipse (rect);
			}
			else
				item.Icon_.paint (painter, rect);

			rect.moveLeft (rect.left () - height - HorizontalPadding);
		}

		painter->restore ();

		return option.rect.right () - rect.right ();
	}
}
}
