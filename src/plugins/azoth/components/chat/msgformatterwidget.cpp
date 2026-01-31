/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "msgformatterwidget.h"
#include <cmath>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QToolBar>
#include <QAction>
#include <QTextBlock>
#include <QColorDialog>
#include <QDomDocument>
#include <QFontDialog>
#include <QActionGroup>
#include <QGridLayout>
#include <QToolButton>
#include <QKeyEvent>
#include <QApplication>
#include <QtDebug>
#include <util/sll/debugprinters.h>
#include <util/sll/qtutil.h>
#include "interfaces/azoth/iresourceplugin.h"
#include "../../xmlsettingsmanager.h"
#include "../../core.h"

namespace LC::Azoth
{
	namespace
	{
		class SmilesTooltip : public QWidget
		{
		public:
			explicit SmilesTooltip (QWidget *parent)
			: QWidget { parent, Qt::Tool }
			{
			}
		protected:
			void keyPressEvent (QKeyEvent *event) override
			{
				if (event->key () == Qt::Key_Escape)
				{
					event->accept ();
					hide ();
				}
				else
					QWidget::keyPressEvent (event);
			}
		};
	}

	auto MsgFormatterWidget::CharFormatter (auto setter)
	{
		return CharFormatter (setter, std::identity {});
	}

	auto MsgFormatterWidget::CharFormatter (auto setter, auto conv)
	{
		return [this, setter, conv] (bool checked)
		{
			if (auto cursor = Edit_.textCursor ();
				cursor.hasSelection ())
			{
				auto fmt = cursor.charFormat ();
				std::invoke (setter, fmt, conv (checked));
				cursor.setCharFormat (fmt);
			}
			else
			{
				auto fmt = Edit_.currentCharFormat ();
				std::invoke (setter, fmt, conv (checked));
				Edit_.setCurrentCharFormat (fmt);
			}

			HasCustomFormatting_ = true;
		};
	}

	MsgFormatterWidget::MsgFormatterWidget (QTextEdit& edit)
	: QWidget { &edit }
	, Edit_ { edit }
	, StockCharFormat_ { Edit_.currentCharFormat () }
	, StockBlockFormat_ { Edit_.document ()->begin ().blockFormat () }
	, StockFrameFormat_ { Edit_.document ()->rootFrame ()->frameFormat () }
	, SmilesTooltip_ { new SmilesTooltip { this } }
	{
		SmilesTooltip_->setWindowTitle (tr ("Emoticons"));

		setLayout (new QVBoxLayout);
		layout ()->setContentsMargins (0, 0, 0, 0);
		const auto toolbar = new QToolBar;
		layout ()->addWidget (toolbar);

		FormatBold_ = toolbar->addAction (tr ("Bold"),
				this,
				CharFormatter (&QTextCharFormat::setFontWeight,
						[] (bool checked) { return checked ? QFont::Bold : QFont::Normal; }));
		FormatBold_->setCheckable (true);
		FormatBold_->setProperty ("ActionIcon", "format-text-bold");

		FormatItalic_ = toolbar->addAction (tr ("Italic"),
				this,
				CharFormatter (&QTextCharFormat::setFontItalic));
		FormatItalic_->setCheckable (true);
		FormatItalic_->setProperty ("ActionIcon", "format-text-italic");

		FormatUnderline_ = toolbar->addAction (tr ("Underline"),
				this,
				CharFormatter (&QTextCharFormat::setFontUnderline));
		FormatUnderline_->setCheckable (true);
		FormatUnderline_->setProperty ("ActionIcon", "format-text-underline");

		FormatStrikeThrough_ = toolbar->addAction (tr ("Strike through"),
				this,
				CharFormatter (&QTextCharFormat::setFontStrikeOut));
		FormatStrikeThrough_->setCheckable (true);
		FormatStrikeThrough_->setProperty ("ActionIcon", "format-text-strikethrough");

		toolbar->addSeparator ();

		const auto formatColor = toolbar->addAction (tr ("Text color"),
				this,
				[this]
				{
					const auto& brush = GetActualCharFormat ().foreground ();
					if (const auto& color = QColorDialog::getColor (brush.color (), &Edit_);
						color.isValid ())
						CharFormatActor ([color] (auto fmt) { fmt->setForeground (QBrush (color)); });
				});
		formatColor->setProperty ("ActionIcon", "format-text-color");

		const auto formatFont = toolbar->addAction (tr ("Font"),
				this,
				[this]
				{
					const auto& curFont = GetActualCharFormat ().font ();
					bool ok = false;
					if (auto font = QFontDialog::getFont (&ok, curFont, &Edit_); ok)
						CharFormatActor ([font] (auto fmt) { fmt->setFont (font); });
				});
		formatFont->setProperty ("ActionIcon", "preferences-desktop-font");

		toolbar->addSeparator ();

		const auto setAlignment = [this] (Qt::Alignment align)
		{
			return [this, align] { BlockFormatActor ([&] (auto fmt) { fmt->setAlignment (align); }); };
		};
		const auto formatAlignLeft = toolbar->addAction (tr ("Align left"),
				this,
				setAlignment (Qt::AlignLeft));
		formatAlignLeft->setProperty ("ActionIcon", "format-justify-left");
		formatAlignLeft->setCheckable (true);
		formatAlignLeft->setChecked (true);

		const auto formatAlignCenter = toolbar->addAction (tr ("Align center"),
				this,
				setAlignment (Qt::AlignCenter));
		formatAlignCenter->setProperty ("ActionIcon", "format-justify-center");
		formatAlignCenter->setCheckable (true);

		const auto formatAlignRight = toolbar->addAction (tr ("Align right"),
				this,
				setAlignment (Qt::AlignRight));
		formatAlignRight->setProperty ("ActionIcon", "format-justify-right");
		formatAlignRight->setCheckable (true);

		const auto formatAlignJustify = toolbar->addAction (tr ("Align justify"),
				this,
				setAlignment (Qt::AlignJustify));
		formatAlignJustify->setProperty ("ActionIcon", "format-justify-fill");
		formatAlignJustify->setCheckable (true);

		const auto alignGroup = new QActionGroup { this };
		alignGroup->addAction (formatAlignLeft);
		alignGroup->addAction (formatAlignCenter);
		alignGroup->addAction (formatAlignRight);
		alignGroup->addAction (formatAlignJustify);

		connect (&Edit_,
				&QTextEdit::currentCharFormatChanged,
				this,
				&MsgFormatterWidget::UpdateState);

		toolbar->addSeparator ();

		AddEmoticon_ = toolbar->addAction (tr ("Emoticons..."),
				this,
				[this]
				{
					SmilesTooltip_->move (QCursor::pos ());
					SmilesTooltip_->show ();
					SmilesTooltip_->activateWindow ();

					const auto& size = SmilesTooltip_->size ();
					const auto& newPos = QCursor::pos () - QPoint (0, size.height ());
					SmilesTooltip_->move (newPos);
				});
		AddEmoticon_->setProperty ("ActionIcon", "face-smile");

		for (const auto act : toolbar->actions ())
			if (!act->isSeparator ())
				act->setParent (this);

		XmlSettingsManager::Instance ().RegisterObject ("SmileIcons", this,
				[this] (const QString& emoPack) { HandleEmoPackChanged (emoPack); });
	}

	bool MsgFormatterWidget::HasCustomFormatting () const
	{
		return HasCustomFormatting_;
	}

	void MsgFormatterWidget::Clear ()
	{
		HasCustomFormatting_ = false;
	}

	std::optional<QString> MsgFormatterWidget::GetNormalizedRichText () const
	{
		if (!HasCustomFormatting ())
			return {};

		auto result = Edit_.toHtml ();

		QDomDocument doc;
		if (const auto parseResult = doc.setContent (result);
			!parseResult)
		{
			qWarning () << "unable to parse" << result << parseResult;
			return {};
		}

		const auto& style = doc.elementsByTagName ("style"_qs).item (0).toElement ();

		auto body = doc.elementsByTagName ("body"_qs).at (0).toElement ();
		const auto& elem = body.firstChildElement ();
		if (elem.isNull ())
			return {};

		body.insertBefore (style.cloneNode (true), elem);
		body.setTagName ("div"_qs);

		QDomDocument finalDoc;
		finalDoc.appendChild (finalDoc.importNode (body, true));

		result = finalDoc.toString ();
		result = result.simplified ();
		result.remove ('\n');
		return result;
	}

	void MsgFormatterWidget::HidePopups ()
	{
		SmilesTooltip_->hide ();
	}

	void MsgFormatterWidget::CharFormatActor (auto format)
	{
		auto cursor = Edit_.textCursor ();
		if (cursor.hasSelection ())
		{
			auto fmt = cursor.charFormat ();
			format (&fmt);
			cursor.setCharFormat (fmt);
		}
		else
		{
			auto fmt = Edit_.currentCharFormat ();
			format (&fmt);
			Edit_.setCurrentCharFormat (fmt);
		}

		HasCustomFormatting_ = true;
	}

	void MsgFormatterWidget::BlockFormatActor (auto format)
	{
		auto fmt = Edit_.textCursor ().blockFormat ();
		format (&fmt);
		Edit_.textCursor ().setBlockFormat (fmt);

		HasCustomFormatting_ = true;
	}

	QTextCharFormat MsgFormatterWidget::GetActualCharFormat () const
	{
		const auto& cursor = Edit_.textCursor ();
		return cursor.hasSelection () ?
				cursor.charFormat () :
				Edit_.currentCharFormat ();
	}

	void MsgFormatterWidget::HandleEmoPackChanged (const QString& emoPack)
	{
		const auto src = Core::Instance ().GetCurrentEmoSource ();
		AddEmoticon_->setEnabled (src);
		if (!src)
			return;

		const auto& images = src->GetReprImages (emoPack);
		if (const auto lay = SmilesTooltip_->layout ())
		{
			while (const auto item = lay->takeAt (0))
				delete item;
			delete lay;
		}

		qDeleteAll (SmilesTooltip_->children ());

		const auto layout = new QGridLayout { SmilesTooltip_ };
		layout->setSpacing (0);
		layout->setContentsMargins (1, 1, 1, 1);
		const int numRows = std::ceil (std::sqrt (static_cast<double> (images.size ())));
		int pos = 0;
		QSize maxSize;
		QList<QToolButton*> buttons;
		for (const auto& [image, smile] : images)
		{
			const auto& size = image.size ();
			maxSize.setWidth (std::max (size.width (), maxSize.width ()));
			maxSize.setHeight (std::max (size.height (), maxSize.height ()));

			const QIcon icon { QPixmap::fromImage (image) };
			const auto action = new QAction { icon, smile, this };
			action->setToolTip (smile);
			connect (action,
					&QAction::triggered,
					this,
					[this, smile]
					{
						Edit_.textCursor ().insertText (smile + ' ');
						if (!(QApplication::keyboardModifiers () & Qt::ControlModifier))
							SmilesTooltip_->hide ();
					});

			const auto button = new QToolButton { SmilesTooltip_ };
			button->setDefaultAction (action);
			buttons << button;

			layout->addWidget (button, pos / numRows, pos % numRows);
			++pos;
		}

		for (const auto button : buttons)
			button->setIconSize (maxSize);

		SmilesTooltip_->setLayout (layout);
		SmilesTooltip_->adjustSize ();
		SmilesTooltip_->setMaximumSize (SmilesTooltip_->sizeHint ());
	}

	void MsgFormatterWidget::UpdateState (const QTextCharFormat& fmt)
	{
		FormatBold_->setChecked (fmt.fontWeight () != QFont::Normal);
		FormatItalic_->setChecked (fmt.fontItalic ());
		FormatUnderline_->setChecked (fmt.fontUnderline ());
		FormatStrikeThrough_->setChecked (fmt.fontStrikeOut ());
	}
}
