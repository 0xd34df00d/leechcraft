/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Like-all
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

#include "terminalwidget.h"
#include <QVBoxLayout>
#include <QEvent>
#include <QKeyEvent>
#include <QMessageBox>

namespace LeechCraft
{
namespace Shaitan
{
	namespace
	{
		class TerminalContainer : public QX11EmbedContainer
		{
		public:
			TerminalContainer (QWidget *parent)
			: QX11EmbedContainer (parent)
			{
			}

			virtual bool eventFilter (QObject *obj, QEvent *event)
			{
				if (obj == this && event->type () == QEvent::KeyPress)
				{
					auto keyEvent = static_cast<QKeyEvent*> (event);
					if (keyEvent->key () == (Qt::Key_PageDown & Qt::Key_Control) ||
						keyEvent->key () == (Qt::Key_PageUp & Qt::Key_Control))
						return false;
				}
				return QX11EmbedContainer::eventFilter (obj, event);
			}
		};
	}

	TerminalWidget::TerminalWidget (const TabClassInfo& tc, QObject *mt)
	: TC_ (tc)
	, ParentMT_ (mt)
	, Embedder_ (new TerminalContainer (this))
	{
		Process_ = new QProcess (this);

		auto lay = new QVBoxLayout;
		setLayout (lay);
		lay->addWidget (Embedder_);

		Embedder_->adjustSize ();

		Embedder_->show ();

		connect (Process_,
			SIGNAL (error (QProcess::ProcessError)),
			this,
			SLOT (gotError ()));

		Process_->start ("xterm",
			{ "-into", QString::number (Embedder_->winId ()) });
	}

	TabClassInfo TerminalWidget::GetTabClassInfo () const
	{
		return TC_;
	}

	QToolBar* TerminalWidget::GetToolBar () const
	{
		return 0;
	}

	QObject* TerminalWidget::ParentMultiTabs ()
	{
		return ParentMT_;
	}

	void TerminalWidget::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	void TerminalWidget::gotError ()
	{
		QMessageBox::critical (this,
		      "LeechCraft",
		      tr ("XTerm has not started: %1.").arg (Process_->errorString()));
	}
}
}
