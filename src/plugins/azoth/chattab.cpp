/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "chattab.h"
#include <QWebFrame>
#include <QWebElement>
#include "interfaces/iclentry.h"
#include "interfaces/imessage.h"
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			QObject *ChatTab::S_ParentMultiTabs_ = 0;

			void ChatTab::SetParentMultiTabs (QObject *obj)
			{
				S_ParentMultiTabs_ = obj;
			}

			ChatTab::ChatTab (const QPersistentModelIndex& idx,
					const QString& variant, QWidget *parent)
			: QWidget (parent)
			, Index_ (idx)
			, Variant_ (variant)
			{
				Ui_.setupUi (this);

				QObject *entryObj = Index_.data (Core::CLREntryObject).value<QObject*> ();
				connect (entryObj,
						SIGNAL (gotMessage (QObject*)),
						this,
						SLOT (handleEntryMessage (QObject*)));

				Plugins::ICLEntry *e = GetEntry ();
				Q_FOREACH (Plugins::IMessage *msg, e->GetAllMessages ())
					AppendMessage (msg);
			}

			QList<QAction*> ChatTab::GetTabBarContextMenuActions () const
			{
				return QList<QAction*> ();
			}

			QObject* ChatTab::ParentMultiTabs () const
			{
				return S_ParentMultiTabs_;
			}

			void ChatTab::NewTabRequested ()
			{
			}

			QToolBar* ChatTab::GetToolBar () const
			{
				return 0;
			}

			void ChatTab::Remove ()
			{
			}

			void ChatTab::on_MsgEdit__returnPressed ()
			{
				if (!Index_.isValid ())
					return;

				QString text = Ui_.MsgEdit_->text ();
				if (text.isEmpty ())
					return;

				Ui_.MsgEdit_->clear ();

				Plugins::ICLEntry *e = GetEntry ();
				QStringList currentVariants = e->Variants ();
				QString variant = currentVariants.contains (Variant_) ?
						Variant_ :
						currentVariants.first ();
				Plugins::IMessage *msg = e->CreateMessage (Plugins::IMessage::MTChat, variant, text);
				msg->Send ();
				AppendMessage (msg);
			}

			void ChatTab::handleEntryMessage (QObject *msgObj)
			{
				Plugins::IMessage *msg = qobject_cast<Plugins::IMessage*> (msgObj);
				if (!msg)
				{
					qWarning () << Q_FUNC_INFO
							<< msgObj
							<< "doesn't implement IMessage"
							<< sender ();
					return;
				}

				AppendMessage (msg);
			}

			Plugins::ICLEntry* ChatTab::GetEntry ()
			{
				if (!Index_.isValid ())
				{
					qWarning () << Q_FUNC_INFO
							<< "stored persistent index is invalid";
					return 0;
				}

				QObject *entryObj = Index_.data (Core::CLREntryObject).value<QObject*> ();
				Plugins::ICLEntry *entry = qobject_cast<Plugins::ICLEntry*> (entryObj);
				if (!entry)
					qWarning () << Q_FUNC_INFO
							<< "object"
							<< entryObj
							<< "from the index"
							<< Index_
							<< "doesn't implement Plugins::ICLEntry";
				return entry;
			}

			void ChatTab::AppendMessage (Plugins::IMessage *msg)
			{
				QString string = QString ("[%1] ")
						.arg (msg->GetDateTime ().time ().toString ());
				switch (msg->GetDirection ())
				{
				case Plugins::IMessage::DIn:
					string.append (msg->OtherPart ()->GetEntryName ());
					string.append (": ");
					break;
				case Plugins::IMessage::DOut:
					string.append ("R: ");
					break;
				}

				string.append (msg->GetBody ());

				QWebElement elem = Ui_.View_->page ()->mainFrame ()->findFirstElement ("body");
				elem.appendInside (QString ("<div>%1</div").arg (string));
			}
		}
	}
}

