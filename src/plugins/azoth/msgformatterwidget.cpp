/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include <QtDebug>
#include "interfaces/iresourceplugin.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	MsgFormatterWidget::MsgFormatterWidget (QTextEdit *edit, QWidget *parent)
	: QWidget (parent)
	, Edit_ (edit)
	, StockCharFormat_ (Edit_->currentCharFormat ())
	, StockBlockFormat_ (Edit_->document ()->begin ().blockFormat ())
	, StockFrameFormat_ (Edit_->document ()->rootFrame ()->frameFormat ())
	, HasCustomFormatting_ (false)
	, SmilesTooltip_ (new QWidget (this, Qt::Tool))
	{
		SmilesTooltip_->setWindowTitle (tr ("Emoticons"));

		setLayout (new QVBoxLayout ());
		layout ()->setContentsMargins (0, 0, 0, 0);
		QToolBar *toolbar = new QToolBar ();
		toolbar->setIconSize (QSize (16, 16));
		layout ()->addWidget (toolbar);

		FormatBold_ = toolbar->addAction (tr ("Bold"),
				this,
				SLOT (handleBold ()));
		FormatBold_->setCheckable (true);
		FormatBold_->setProperty ("ActionIcon", "format_text_bold");

		FormatItalic_ = toolbar->addAction (tr ("Italic"),
				this,
				SLOT (handleItalic ()));
		FormatItalic_->setCheckable (true);
		FormatItalic_->setProperty ("ActionIcon", "format_text_italic");

		FormatUnderline_ = toolbar->addAction (tr ("Underline"),
				this,
				SLOT (handleUnderline ()));
		FormatUnderline_->setCheckable (true);
		FormatUnderline_->setProperty ("ActionIcon", "format_text_underline");

		FormatStrikeThrough_ = toolbar->addAction (tr ("Strike through"),
				this,
				SLOT (handleStrikeThrough ()));
		FormatStrikeThrough_->setCheckable (true);
		FormatStrikeThrough_->setProperty ("ActionIcon", "format_text_strikethrough");

		toolbar->addSeparator ();

		FormatColor_ = toolbar->addAction (tr ("Text color"),
				this,
				SLOT (handleTextColor ()));
		FormatColor_->setProperty ("ActionIcon", "format_text_color");

		FormatFont_ = toolbar->addAction (tr ("Font"),
				this,
				SLOT (handleFont ()));
		FormatFont_->setProperty ("ActionIcon", "format_text_font");

		toolbar->addSeparator ();

		FormatAlignLeft_ = toolbar->addAction (tr ("Align left"),
				this,
				SLOT (handleParaAlignment ()));
		FormatAlignLeft_->setProperty ("ActionIcon", "format_text_alignleft");
		FormatAlignLeft_->setProperty ("Alignment", static_cast<int> (Qt::AlignLeft));
		FormatAlignLeft_->setCheckable (true);
		FormatAlignLeft_->setChecked (true);

		FormatAlignCenter_ = toolbar->addAction (tr ("Align center"),
				this,
				SLOT (handleParaAlignment ()));
		FormatAlignCenter_->setProperty ("ActionIcon", "format_text_aligncenter");
		FormatAlignCenter_->setProperty ("Alignment", static_cast<int> (Qt::AlignCenter));
		FormatAlignCenter_->setCheckable (true);

		FormatAlignRight_ = toolbar->addAction (tr ("Align right"),
				this,
				SLOT (handleParaAlignment ()));
		FormatAlignRight_->setProperty ("ActionIcon", "format_text_alignright");
		FormatAlignRight_->setProperty ("Alignment", static_cast<int> (Qt::AlignRight));
		FormatAlignRight_->setCheckable (true);

		FormatAlignJustify_ = toolbar->addAction (tr ("Align justify"),
				this,
				SLOT (handleParaAlignment ()));
		FormatAlignJustify_->setProperty ("ActionIcon", "format_text_alignjustify");
		FormatAlignJustify_->setProperty ("Alignment", static_cast<int> (Qt::AlignJustify));
		FormatAlignJustify_->setCheckable (true);

		QActionGroup *alignGroup = new QActionGroup (this);
		alignGroup->addAction (FormatAlignLeft_);
		alignGroup->addAction (FormatAlignCenter_);
		alignGroup->addAction (FormatAlignRight_);
		alignGroup->addAction (FormatAlignJustify_);

		connect (Edit_,
				SIGNAL (currentCharFormatChanged (const QTextCharFormat&)),
				this,
				SLOT (updateState (const QTextCharFormat&)));
		connect (Edit_,
				SIGNAL (textChanged ()),
				this,
				SLOT (checkCleared ()));

		toolbar->addSeparator ();

		AddEmoticon_ = toolbar->addAction (tr ("Emoticons..."),
				this,
				SLOT (handleAddEmoticon ()));
		AddEmoticon_->setProperty ("ActionIcon", "emoticons");

		XmlSettingsManager::Instance ().RegisterObject ("SmileIcons",
				this, "handleEmoPackChanged");
		handleEmoPackChanged ();
	}

	bool MsgFormatterWidget::HasCustomFormatting () const
	{
		return HasCustomFormatting_;
	}

	void MsgFormatterWidget::Clear ()
	{
		HasCustomFormatting_ = false;
	}

	QString MsgFormatterWidget::GetNormalizedRichText () const
	{
		if (!HasCustomFormatting ())
			return QString ();

		QString result = Edit_->toHtml ();

		QDomDocument doc;
		if (!doc.setContent (result))
			return result;

		const QDomNodeList& styles = doc.elementsByTagName ("style");
		const QDomElement& style = styles.size () ?
				styles.at (0).toElement () :
				QDomElement ();

		QDomElement body = doc.elementsByTagName ("body").at (0).toElement ();
		const QDomElement& elem = body.firstChildElement ();
		if (elem.isNull ())
			return QString ();
		else
			body.insertBefore (style.cloneNode (true), elem);

		body.setTagName ("div");

		QDomDocument finalDoc;
		finalDoc.appendChild (finalDoc.importNode (body, true));

		result = finalDoc.toString ();
		result = result.simplified ();
		result.remove ('\n');
		return result;
	}

	void MsgFormatterWidget::CharFormatActor (std::function<void (QTextCharFormat*)> format)
	{
		QTextCursor cursor = Edit_->textCursor ();
		if (cursor.hasSelection ())
		{
			QTextCharFormat fmt = cursor.charFormat ();
			format (&fmt);
			cursor.setCharFormat (fmt);
		}
		else
		{
			QTextCharFormat fmt = Edit_->currentCharFormat ();
			format (&fmt);
			Edit_->setCurrentCharFormat (fmt);
		}

		HasCustomFormatting_ = true;
	}

	void MsgFormatterWidget::BlockFormatActor (std::function<void (QTextBlockFormat*)> format)
	{
		QTextBlockFormat fmt = Edit_->textCursor ().blockFormat ();
		format (&fmt);
		Edit_->textCursor ().setBlockFormat (fmt);

		HasCustomFormatting_ = true;
	}

	QTextCharFormat MsgFormatterWidget::GetActualCharFormat () const
	{
		const QTextCursor& cursor = Edit_->textCursor ();
		return cursor.hasSelection () ?
				cursor.charFormat () :
				Edit_->currentCharFormat ();
	}

	void MsgFormatterWidget::handleBold ()
	{
		CharFormatActor ([FormatBold_] (QTextCharFormat *fmt)
				{ fmt->setFontWeight (FormatBold_->isChecked () ? QFont::Bold : QFont::Normal); });
	}

	void MsgFormatterWidget::handleItalic ()
	{
		CharFormatActor ([FormatItalic_] (QTextCharFormat *fmt)
				{ fmt->setFontItalic (FormatItalic_->isChecked ()); });
	}

	void MsgFormatterWidget::handleUnderline ()
	{
		CharFormatActor ([FormatUnderline_] (QTextCharFormat *fmt)
				{ fmt->setFontUnderline (FormatUnderline_->isChecked ()); });
	}

	void MsgFormatterWidget::handleStrikeThrough ()
	{
		CharFormatActor ([FormatStrikeThrough_] (QTextCharFormat *fmt)
				{ fmt->setFontStrikeOut (FormatStrikeThrough_->isChecked ()); });
	}

	void MsgFormatterWidget::handleTextColor ()
	{
		QBrush brush = GetActualCharFormat ().foreground ();
		const QColor& color = QColorDialog::getColor (brush.color (), Edit_);
		if (!color.isValid ())
			return;

		CharFormatActor ([color] (QTextFormat *fmt)
				{ fmt->setForeground (QBrush (color)); });
	}

	void MsgFormatterWidget::handleFont ()
	{
		QFont font = GetActualCharFormat ().font ();
		bool ok = false;
		font = QFontDialog::getFont (&ok, font, Edit_);
		if (!ok)
			return;

		CharFormatActor ([font] (QTextCharFormat *fmt)
				{ fmt->setFont (font); });
	}

	void MsgFormatterWidget::handleParaAlignment ()
	{
		Qt::Alignment alignment = static_cast<Qt::Alignment> (sender ()->
					property ("Alignment").toInt ());
		BlockFormatActor ([alignment] (QTextBlockFormat* fmt)
				{ fmt->setAlignment (alignment); });
	}

	void MsgFormatterWidget::handleAddEmoticon ()
	{
		if (!SmilesTooltip_)
			return;

		SmilesTooltip_->move (QCursor::pos ());
		SmilesTooltip_->show ();

		const QSize& size = SmilesTooltip_->size ();
		const QPoint& newPos = QCursor::pos () - QPoint (0, size.height ());
		SmilesTooltip_->move (newPos);
	}

	void MsgFormatterWidget::handleEmoPackChanged ()
	{
		const QString& emoPack = XmlSettingsManager::Instance ()
				.property ("SmileIcons").toString ();
		AddEmoticon_->setEnabled (!emoPack.isEmpty ());

		IEmoticonResourceSource *src = Core::Instance ().GetCurrentEmoSource ();
		if (!src)
			return;

		const QHash<QImage, QString>& images = src->GetReprImages (emoPack);

		QLayout *lay = SmilesTooltip_->layout ();
		if (lay)
		{
			while (lay->count ())
				delete lay->takeAt (0);
			delete lay;
		}

		QGridLayout *layout = new QGridLayout (SmilesTooltip_);
		layout->setSpacing (0);
		layout->setContentsMargins (1, 1, 1, 1);
		const int numRows = std::sqrt (static_cast<double> (images.size ())) + 1;
		int pos = 0;
		for (QHash<QImage, QString>::const_iterator i = images.begin (),
				end = images.end (); i != end; ++i)
		{
			const QIcon icon (QPixmap::fromImage (i.key ()));
			QAction *action = new QAction (icon, *i, this);
			action->setToolTip (*i);
			action->setProperty ("Text", *i);

			connect (action,
					SIGNAL (triggered ()),
					this,
					SLOT (insertEmoticon ()));

			QToolButton *button = new QToolButton ();
			button->setDefaultAction (action);

			layout->addWidget (button, pos / numRows, pos % numRows);
			++pos;
		}

		SmilesTooltip_->setLayout (layout);
	}

	void MsgFormatterWidget::insertEmoticon ()
	{
		const QString& text = sender ()->property ("Text").toString ();
		Edit_->textCursor ().insertText (text + " ");

		if (SmilesTooltip_)
			SmilesTooltip_->hide ();
	}

	void MsgFormatterWidget::checkCleared ()
	{
		if (Edit_->toPlainText ().simplified ().isEmpty ())
			updateState (Edit_->currentCharFormat ());
	}

	void MsgFormatterWidget::updateState (const QTextCharFormat& fmt)
	{
		FormatBold_->setChecked (fmt.fontWeight () != QFont::Normal);
		FormatItalic_->setChecked (fmt.fontItalic ());
		FormatUnderline_->setChecked (fmt.fontUnderline ());
		FormatStrikeThrough_->setChecked (fmt.fontStrikeOut ());
	}
}
}
