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
#include <QTextDocument>
#include <QTimer>
#include <QPalette>
#include <QApplication>
#include <plugininterface/defaulthookproxy.h>
#include <plugininterface/util.h>
#include "interfaces/iclentry.h"
#include "interfaces/imessage.h"
#include "core.h"
#include "interfaces/iaccount.h"
#include <boost/graph/graph_concepts.hpp>

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
			, LinkRegexp_ ("(\\b(?:(?:https?|ftp)://|www.|xmpp:)[\\w\\d/\\?.=:@&%#_;\\(?:\\)\\+\\-\\~\\*\\,]+)",
					Qt::CaseInsensitive, QRegExp::RegExp2)
			, BgColor_ (QApplication::palette ().color (QPalette::Base))
			{
				Ui_.setupUi (this);
				Ui_.View_->page ()->setLinkDelegationPolicy (QWebPage::DelegateAllLinks);
				QFile file (":/plugins/azoth/resources/html/viewcontents.html");
				if (!file.open (QIODevice::ReadOnly))
				{
					qWarning () << Q_FUNC_INFO
							<< "could not open resource file"
							<< file.errorString ();
				}
				else
				{
					QString data = file.readAll ();
					data.replace ("BACKGROUNDCOLOR",
							BgColor_.name ());
					data.replace ("FOREGROUNDCOLOR",
							QApplication::palette ().color (QPalette::Text).name ());
					Ui_.View_->setHtml (data);
				}

				connect (Ui_.View_->page (),
						SIGNAL (linkClicked (const QUrl&)),
						this,
						SLOT (handleViewLinkClicked (const QUrl&)));

				QObject *entryObj = Index_.data (Core::CLREntryObject).value<QObject*> ();
				connect (entryObj,
						SIGNAL (gotMessage (QObject*)),
						this,
						SLOT (handleEntryMessage (QObject*)));

				Plugins::ICLEntry *e = GetEntry ();
				Q_FOREACH (Plugins::IMessage *msg, e->GetAllMessages ())
					AppendMessage (msg);

				Ui_.MsgEdit_->setFocus ();
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
				emit needToClose (this);
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
				Plugins::IMessage::MessageType type =
						e->GetEntryFeatures () & Plugins::ICLEntry::FIsMUC ?
								Plugins::IMessage::MTMUCMessage :
								Plugins::IMessage::MTChatMessage;
				Plugins::IMessage *msg = e->CreateMessage (type, variant, text);
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

			void ChatTab::handleViewLinkClicked (const QUrl& url)
			{
				qDebug () << url.scheme () << url.host () << url.path ();
				if (url.scheme () != "azoth")
				{
					Entity e = Util::MakeEntity (url,
							QString (), FromUserInitiated);
					Core::Instance ().SendEntity (e);
					return;
				}

				if (url.host () == "msgeditreplace")
				{
					Ui_.MsgEdit_->setText (url.path ().mid (1));
					Ui_.MsgEdit_->setFocus ();
				}
			}

			void ChatTab::scrollToEnd()
			{
				QWebFrame *frame = Ui_.View_->page ()->mainFrame ();
				int scrollMax = frame->scrollBarMaximum (Qt::Vertical);
				frame->setScrollBarValue (Qt::Vertical, scrollMax);
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
				if (msg->GetDirection () == Plugins::IMessage::DOut &&
						msg->OtherPart ()->GetEntryFeatures () & Plugins::ICLEntry::FIsMUC)
					return;

				QWebFrame *frame = Ui_.View_->page ()->mainFrame ();
				bool shouldScrollFurther = (frame->scrollBarMaximum (Qt::Vertical) ==
								frame->scrollBarValue (Qt::Vertical));

				QString body = FormatBody (msg->GetBody (), msg);

				QString string = QString ("%1 ")
						.arg (FormatDate (msg->GetDateTime (), msg));
				string.append (' ');
				switch (msg->GetDirection ())
				{
				case Plugins::IMessage::DIn:
				{
					QString entryName = Qt::escape (msg->OtherPart ()->GetEntryName ());
					entryName = FormatNickname (entryName, msg);

					if (body.startsWith ("/me "))
					{
						body = body.mid (3);
						string.append ("*** ");
						string.append (entryName);
						string.append (' ');
					}
					else
					{
						string.append (entryName);
						string.append (": ");
					}
					break;
				}
				case Plugins::IMessage::DOut:
					string.append (FormatNickname ("R", msg));
					string.append (": ");
					break;
				}

				string.append (body);

				QWebElement elem = frame->findFirstElement ("body");
				elem.appendInside (QString ("<div>%1</div").arg (string));

				if (shouldScrollFurther)
					QTimer::singleShot (100,
							this,
							SLOT (scrollToEnd ()));
			}

			QString ChatTab::FormatDate (QDateTime dt, Plugins::IMessage *msg)
			{
				Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
				emit hookFormatDateTime (proxy, &dt, msg->GetObject ());
				if (proxy->IsCancelled ())
					return proxy->GetReturnValue ().toString ();

				QString str = dt.time ().toString ();
				return QString ("<span class='datetime' style='color:green'>[" +
						str + "]</span>");
			}

			QString ChatTab::FormatNickname (QString nick, Plugins::IMessage *msg)
			{
				Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
				emit hookFormatNickname (proxy, &nick, msg->GetObject ());
				if (proxy->IsCancelled ())
					return nick;

				QString string;

				QString color = "green";
				if (msg->GetMessageType () == Plugins::IMessage::MTMUCMessage)
				{
					if (NickColors_.isEmpty ())
						GenerateColors ();

					if (!NickColors_.isEmpty ())
					{
						int hash = 0;
						for (int i = 0; i < nick.length (); ++i)
							hash += nick.at (i).unicode ();
						QColor nc = NickColors_.at (hash % NickColors_.size ());
						color = nc.name ();
					}

					QUrl url (QString ("azoth://msgeditreplace/%1")
							.arg (nick + ":"));

					string.append ("<span class='nickname'><a href='");
					string.append (url.toString () + "%20");
					string.append ("' class='nicklink' style='text-decoration:none; color:");
					string.append (color);
					string.append ("'>");
					string.append (nick);
					string.append ("</a></span>");
				}
				else
				{
					switch (msg->GetDirection ())
					{
						case Plugins::IMessage::DIn:
							color = "blue";
							break;
						case Plugins::IMessage::DOut:
							color = "red";
							break;
					}

					string = QString ("<span class='nickname' style='color:%2'>%1</span>")
							.arg (nick)
							.arg (color);
				}

				return string;
			}

			QString ChatTab::FormatBody (QString body, Plugins::IMessage *msg)
			{
				QObject *msgObj = msg->GetObject ();

				Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
				emit hookFormatBodyBegin (proxy, &body, msgObj);
				if (!proxy->IsCancelled ())
				{
					body = Qt::escape (body);
					body.replace ('\n', "<br />");
					body.replace ("  ", "&nbsp; ");

					int pos = 0;
					while ((pos = LinkRegexp_.indexIn (body, pos)) != -1)
					{
						QString link = LinkRegexp_.cap (1);
						QString str = QString ("<a href=\"%1\">%1</a>")
								.arg (link);
						body.replace (pos, link.length (), str);

						pos += str.length ();
					}

					emit hookFormatBodyEnd (proxy, &body, msgObj);
				}

				return proxy->IsCancelled () ?
						proxy->GetReturnValue ().toString () :
						body;
			}

			namespace
			{
				qreal Fix (qreal h)
				{
					while (h < 0)
						h += 1;
					while (h >= 1)
						h -= 1;
					return h;
				}
			}

			void ChatTab::GenerateColors ()
			{
				const qreal lower = 50. / 360.;
				const qreal higher = 160. / 360.;
				const qreal delta = 25. / 360.;

				const qreal alpha = BgColor_.alphaF ();

				qreal s = BgColor_.saturationF ();
				s += (1 - s) / 2;
				qreal v = BgColor_.valueF ();
				v = 0.95 - v / 2;

				qreal h = BgColor_.hueF ();

				QColor color;
				for (qreal d = lower; d <= higher; d += delta)
				{
					color.setHsvF (Fix (h + d), s, v, alpha);
					NickColors_ << color;
					color.setHsvF (Fix (h - d), s, v, alpha);
					NickColors_ << color;
				}
			}
		}
	}
}
