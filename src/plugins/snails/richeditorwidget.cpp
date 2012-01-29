/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "richeditorwidget.h"
#include <QWebFrame>
#include <QWebPage>
#include <QWebElement>
#include <QToolBar>
#include <QMenu>
#include <QtDebug>
#include "core.h"

namespace LeechCraft
{
namespace Snails
{
	namespace
	{
		class Addable
		{
			enum class Type
			{
				Menu,
				Bar
			} Type_;

			QMenu *Menu_;
			QToolBar *Bar_;
		public:
			explicit Addable (QMenu *menu)
			: Type_ (Type::Menu)
			, Menu_ (menu)
			{
			}

			explicit Addable (QToolBar *bar)
			: Type_ (Type::Bar)
			, Bar_ (bar)
			{
			}

			QAction *addAction (const QString& text, const QObject *receiver, const char *member)
			{
				switch (Type_)
				{
				case Type::Menu:
					return Menu_->addAction (text, receiver, member);
				case Type::Bar:
					return Bar_->addAction (text, receiver, member);
				}
			}
		};
	}

	RichEditorWidget::RichEditorWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		Ui_.View_->page ()->setContentEditable (true);
		connect (Ui_.View_->page (),
				SIGNAL (selectionChanged ()),
				this,
				SLOT (updateActions ()));

		QToolBar *bar = new QToolBar (tr ("Editor bar"));
		qobject_cast<QVBoxLayout*> (layout ())->insertWidget (0, bar);

		auto fwdCmd = [bar, this] (const QString& name,
				const QString& icon,
				QWebPage::WebAction action,
				Addable addable)
		{
			QAction *act = addable.addAction (name,
					Ui_.View_->pageAction (action), SLOT (trigger ()));
			act->setProperty ("ActionIcon", icon);
			connect (Ui_.View_->pageAction (action),
					SIGNAL (changed ()),
					this,
					SLOT (updateActions ()));

			WebAction2Action_ [action] = act;
		};
		auto addCmd = [bar, this] (const QString& name,
				const QString& icon,
				const QString& cmd,
				Addable addable,
				const QString& arg = QString ())
		{
			QAction *act = addable.addAction (name, this, SLOT (handleCmd ()));
			act->setProperty ("ActionIcon", icon);
			act->setProperty ("Editor/Command", cmd);
			act->setProperty ("Editor/Args", arg);

			Cmd2Action_ [cmd] [arg] = act;
		};

		Addable barAdd (bar);

		fwdCmd (tr ("Bold"), "format-text-bold", QWebPage::ToggleBold, barAdd);
		fwdCmd (tr ("Italic"), "format-text-italic", QWebPage::ToggleItalic, barAdd);
		fwdCmd (tr ("Underline"), "format-text-underline", QWebPage::ToggleUnderline, barAdd);
		addCmd (tr ("Strikethrough"), "format-text-strikethrough", "strikeThrough", barAdd);

		bar->addSeparator ();

		fwdCmd (tr ("Align left"), "format-justify-left", QWebPage::AlignLeft, barAdd);
		fwdCmd (tr ("Align center"), "format-justify-center", QWebPage::AlignCenter, barAdd);
		fwdCmd (tr ("Align right"), "format-justify-right", QWebPage::AlignRight, barAdd);
		fwdCmd (tr ("Align justify"), "format-justify-fill", QWebPage::AlignJustified, barAdd);

		bar->addSeparator ();

		QMenu *headMenu = new QMenu (tr ("Headings"));
		headMenu->setIcon (Core::Instance ().GetProxy ()->GetIcon ("view-list-details"));
		bar->addAction (headMenu->menuAction ());
		for (int i = 1; i <= 6; ++i)
		{
			const auto& num = QString::number (i);
			addCmd (tr ("Heading %1").arg (i), QString (), "formatBlock",
					Addable (headMenu), "h" + num);
		}

		bar->addSeparator ();

		addCmd (tr ("Indent more"), "format-indent-more", "indent", barAdd);
		addCmd (tr ("Indent less"), "format-indent-less", "outdent", barAdd);

		bar->addSeparator ();

		addCmd (tr ("Ordered list"), "format-list-ordered", "insertOrderedList", barAdd);
		addCmd (tr ("Unordered list"), "format-list-unordered", "insertUnorderedList", barAdd);
	}

	QString RichEditorWidget::GetHTML () const
	{
		return Ui_.View_->page ()->mainFrame ()->toHtml ();
	}

	QString RichEditorWidget::GetPlainText () const
	{
		return Ui_.View_->page ()->mainFrame ()->toPlainText ();
	}

	void RichEditorWidget::ExecCommand (const QString& cmd, const QString& arg)
	{
		auto frame = Ui_.View_->page ()->mainFrame ();
		const QString& js = arg.isEmpty () ?
				QString ("document.execCommand(\"%1\", false, null)").arg (cmd) :
				QString ("document.execCommand(\"%1\", false, \"%2\")").arg (cmd, arg);
		frame->evaluateJavaScript (js);

		qDebug () << GetHTML ();
	}

	bool RichEditorWidget::QueryCommandState (const QString& cmd)
	{
		auto frame = Ui_.View_->page ()->mainFrame ();
		const QString& js = QString ("document.queryCommandState(\"%1\", false, null)").arg (cmd);
		auto res = frame->evaluateJavaScript (js);
		return res.toString ().simplified ().toLower () == "true";
	}

	void RichEditorWidget::updateActions ()
	{
		auto upAct = [this] (const QString& cmd, const QString& arg = QString ())
		{
			Cmd2Action_ [cmd] [arg]->setChecked (QueryCommandState (cmd));
		};

		upAct ("strikeThrough");
		//upAct ("insertOrderedList");
		//upAct ("insertUnorderedList");

		auto upWebAct = [this] (QWebPage::WebAction action)
		{
			WebAction2Action_ [action]->setChecked (Ui_.View_->pageAction (action)->isChecked ());
		};

		upWebAct (QWebPage::ToggleBold);
		upWebAct (QWebPage::ToggleItalic);
		upWebAct (QWebPage::ToggleUnderline);

		upWebAct (QWebPage::AlignLeft);
		upWebAct (QWebPage::AlignCenter);
		upWebAct (QWebPage::AlignRight);
		upWebAct (QWebPage::AlignJustified);
	}

	void RichEditorWidget::handleCmd ()
	{
		ExecCommand (sender ()->property ("Editor/Command").toString (),
				sender ()->property ("Editor/Args").toString ());
	}
}
}
