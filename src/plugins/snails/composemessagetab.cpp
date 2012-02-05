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

#include "composemessagetab.h"
#include <QToolBar>
#include <QWebFrame>
#include <QMenu>
#include <interfaces/itexteditor.h>
#include <interfaces/core/ipluginsmanager.h>
#include "message.h"
#include "core.h"

namespace LeechCraft
{
namespace Snails
{
	QObject *ComposeMessageTab::S_ParentPlugin_ = 0;
	TabClassInfo ComposeMessageTab::S_TabClassInfo_;

	void ComposeMessageTab::SetParentPlugin (QObject *obj)
	{
		S_ParentPlugin_ = obj;
	}

	void ComposeMessageTab::SetTabClassInfo (const TabClassInfo& info)
	{
		S_TabClassInfo_ = info;
	}

	ComposeMessageTab::ComposeMessageTab (QWidget *parent)
	: QWidget (parent)
	, Toolbar_ (new QToolBar (tr ("Compose tab bar")))
	, MsgEditWidget_ (0)
	, MsgEdit_ (0)
	{
		Ui_.setupUi (this);

		QAction *send = new QAction (tr ("Send"), this);
		send->setProperty ("ActionIcon", "mail-send");
		connect (send,
				SIGNAL (triggered ()),
				this,
				SLOT (handleSend ()));
		Toolbar_->addAction (send);

		AccountsMenu_ = new QMenu (tr ("Accounts"));
		AccountsMenu_->menuAction ()->setProperty ("ActionIcon", "system-users");
		QActionGroup *accsGroup = new QActionGroup (this);
		Q_FOREACH (Account_ptr account, Core::Instance ().GetAccounts ())
		{
			QAction *act = new QAction (account->GetName (), this);
			accsGroup->addAction (act);
			act->setCheckable (true);
			act->setChecked (true);
			act->setProperty ("Account", QVariant::fromValue<Account_ptr> (account));

			AccountsMenu_->addAction (act);
		}
		Toolbar_->addAction (AccountsMenu_->menuAction ());

		QVBoxLayout *editFrameLay = new QVBoxLayout ();
		editFrameLay->setContentsMargins (0, 0, 0, 0);
		Ui_.MsgEditFrame_->setLayout (editFrameLay);

		auto plugs = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<ITextEditor*> ();
		Q_FOREACH (ITextEditor *plug, plugs)
		{
			if (!plug->SupportsEditor (ContentType::PlainText))
				continue;

			QWidget *w = plug->GetTextEditor (ContentType::PlainText);
			MsgEdit_ = qobject_cast<IEditorWidget*> (w);
			if (!MsgEdit_)
			{
				delete w;
				continue;
			}

			MsgEditWidget_ = w;
			editFrameLay->addWidget (w);
		}
	}

	TabClassInfo ComposeMessageTab::GetTabClassInfo () const
	{
		return S_TabClassInfo_;
	}

	QObject* ComposeMessageTab::ParentMultiTabs ()
	{
		return S_ParentPlugin_;
	}

	void ComposeMessageTab::Remove ()
	{
		emit removeTab (this);
		delete MsgEditWidget_;
		deleteLater ();
	}

	QToolBar* ComposeMessageTab::GetToolBar () const
	{
		return Toolbar_;
	}

	namespace
	{
		QList<QPair<QString, QString>> FromUserInput (const QString& text)
		{
			QList<QPair<QString, QString>> result;
			const QStringList& split = text.split (',', QString::SkipEmptyParts);

			Q_FOREACH (QString address, split)
			{
				address = address.trimmed ();

				QString name;

				const int idx = address.lastIndexOf (' ');
				if (idx > 0)
				{
					name = address.left (idx).trimmed ();
					address = address.mid (idx).simplified ();
				}

				if (address.startsWith ('<') &&
						address.endsWith ('>'))
				{
					address = address.mid (1);
					address.chop (1);
				}

				result.append ({ name, address });
			}

			return result;
		}
	}

	void ComposeMessageTab::handleSend ()
	{
		Account_ptr account;
		Q_FOREACH (QAction *act, AccountsMenu_->actions ())
		{
			if (!act->isChecked ())
				continue;

			account = act->property ("Account").value<Account_ptr> ();
			break;
		}
		if (!account)
			return;

		Message_ptr message (new Message);
		message->SetTo (FromUserInput (Ui_.To_->text ()));
		message->SetSubject (Ui_.Subject_->text ());
		message->SetBody (MsgEdit_->GetContents (ContentType::PlainText));
		message->SetHTMLBody (MsgEdit_->GetContents (ContentType::HTML));

		account->SendMessage (message);
	}
}
}
