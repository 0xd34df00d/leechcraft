/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "regformhandlerwidget.h"
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QLineEdit>
#include <QDomElement>
#include <QtDebug>
#include "xmppbobmanager.h"
#include "util.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	const QString NsRegister = "jabber:iq:register";

	RegFormHandlerWidget::RegFormHandlerWidget (QXmppClient *client, QWidget *parent)
	: QWidget (parent)
	, Client_ (client)
	, BobManager_ (client->findExtension<XMPPBobManager> ())
	, FB_ (FormBuilder (QString (), BobManager_))
	, Widget_ (0)
	, State_ (State::Idle)
	{
		setLayout (new QVBoxLayout);

		connect (Client_,
				SIGNAL (error (QXmppClient::Error)),
				this,
				SLOT (handleError (QXmppClient::Error)));
		connect (Client_,
				SIGNAL (iqReceived (const QXmppIq&)),
				this,
				SLOT (handleIqReceived (const QXmppIq&)));
	}

	QString RegFormHandlerWidget::GetUser () const
	{
		return FormType_ == FTNew ?
				FB_.GetSavedUsername () :
				LFB_.GetUsername ();
	}

	QString RegFormHandlerWidget::GetPassword () const
	{
		return FormType_ == FTNew ?
				FB_.GetSavedPass () :
				LFB_.GetPassword ();
	}

	bool RegFormHandlerWidget::IsComplete () const
	{
		switch (State_)
		{
		case State::Error:
		case State::Idle:
		case State::Connecting:
		case State::FetchingForm:
		case State::AwaitingRegistrationResult:
			return false;
		case State::AwaitingUserInput:
			if (!Widget_)
				return false;
			for (QLineEdit *edit : Widget_->findChildren<QLineEdit*> ())
				if (edit->text ().isEmpty () && edit->property ("Azoth/Xoox/IsRequired").toBool ())
					return false;
			return true;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown state"
				<< static_cast<int> (State_);
		return false;
	}

	void RegFormHandlerWidget::HandleConnecting (const QString& server)
	{
		ShowMessage (tr ("Connecting to %1...").arg (server));
		SetState (State::Connecting);
	}

	void RegFormHandlerWidget::SendRequest (const QString& jid)
	{
		ShowMessage ("Fetching registration form...");

		ReqJID_ = jid;

		QXmppElement queryElem;
		queryElem.setTagName ("query");
		queryElem.setAttribute ("xmlns", NsRegister);

		QXmppIq iq;
		iq.setExtensions (QXmppElementList () << queryElem);
		if (!jid.isEmpty ())
			iq.setTo (jid);
		Client_->sendPacket (iq);
		LastStanzaID_ = iq.id ();

		SetState (State::FetchingForm);
	}

	void RegFormHandlerWidget::Register ()
	{
		QXmppElement queryElem;
		queryElem.setTagName ("query");
		queryElem.setAttribute ("xmlns", NsRegister);

		switch (FormType_)
		{
		case FTLegacy:
			Q_FOREACH (const QXmppElement& elem, LFB_.GetFilledChildren ())
				queryElem.appendChild (elem);
			break;
		case FTNew:
		{
			QByteArray arr;
			{
				QXmlStreamWriter w (&arr);
				FB_.GetForm ().toXml (&w);
			}
			QDomDocument dom;
			dom.setContent (arr);
			queryElem.appendChild (QXmppElement (dom.documentElement ()));
			break;
		}
		}

		QXmppIq iq (QXmppIq::Set);
		if (!ReqJID_.isEmpty ())
			iq.setTo (ReqJID_);
		iq.setExtensions (QXmppElementList () <<  queryElem);
		Client_->sendPacket (iq);
		LastStanzaID_ = iq.id ();

		SetState (State::AwaitingRegistrationResult);
	}

	void RegFormHandlerWidget::Clear ()
	{
		auto widgets = findChildren<QWidget*> ();
		QList<QPointer<QWidget>> pWidgets;
		std::transform (widgets.begin (), widgets.end (), std::back_inserter (pWidgets),
				[] (QWidget *w) { return QPointer<QWidget> (w); });
		Q_FOREACH (auto pWidget, pWidgets)
			if (pWidget)
				delete pWidget;
	}

	void RegFormHandlerWidget::ShowMessage (const QString& msg)
	{
		Clear ();

		layout ()->addWidget (new QLabel (msg));
	}

	void RegFormHandlerWidget::SetState (RegFormHandlerWidget::State state)
	{
		State_ = state;
		emit completeChanged ();
	}

	void RegFormHandlerWidget::HandleRegForm (const QXmppIq& iq)
	{
		QXmppElement queryElem;
		Q_FOREACH (const QXmppElement& elem, iq.extensions ())
		{
			if (elem.tagName () == "query" &&
					elem.attribute ("xmlns") == NsRegister)
			{
				queryElem = elem;
				break;
			}
		}

		if (queryElem.isNull ())
		{
			SetState (State::Error);
			ShowMessage (tr ("Service unavailable"));
			return;
		}

		Clear ();

		const QXmppElement& formElem = queryElem.firstChildElement ("x");
		if ((formElem.attribute ("xmlns") == NsRegister ||
					formElem.attribute ("xmlns") == "jabber:x:data") &&
				formElem.attribute ("type") == "form")
		{
			QXmppDataForm form;
			form.parse (XooxUtil::XmppElem2DomElem (formElem));
			Widget_ = FB_.CreateForm (form);
			FormType_ = FTNew;
		}
		else
		{
			Widget_ = LFB_.CreateForm (queryElem);
			FormType_ = FTLegacy;
		}

		if (!Widget_)
		{
			SetState (State::Error);
			qWarning () << Q_FUNC_INFO
					<< "got null widget";
			return;
		}

		layout ()->addWidget (Widget_);
		Q_FOREACH (QLineEdit *edit, Widget_->findChildren<QLineEdit*> ())
			connect (edit,
					SIGNAL (textChanged (const QString&)),
					this,
					SIGNAL (completeChanged ()));

		SetState (State::AwaitingUserInput);
	}

	void RegFormHandlerWidget::HandleRegResult (const QXmppIq& iq)
	{
		if (iq.type () == QXmppIq::Result)
		{
			emit successfulReg ();
			return;
		}
		else if (iq.type () != QXmppIq::Error)
		{
			qWarning () << Q_FUNC_INFO
					<< "strange iq type"
					<< iq.type ();
			return;
		}

		QString msg;
		Q_FOREACH (const QXmppElement& elem, iq.extensions ())
		{
			if (elem.tagName () != "error")
				continue;

			if (!elem.firstChildElement ("conflict").isNull ())
				msg = tr ("data conflict");
			else if (!elem.firstChildElement ("not-acceptable").isNull ())
				msg = tr ("data is not acceptable");
			else
				msg = tr ("general error:") +
					' ' + elem.firstChildElement ().tagName ();
		}
		if (msg.isEmpty ())
			msg = tr ("general registration error");
		emit regError (msg);
	}

	void RegFormHandlerWidget::handleError (QXmppClient::Error error)
	{
		QString msg;
		switch (error)
		{
		case QXmppClient::SocketError:
			msg = tr ("Socket error:") + ' ' + QString::number (Client_->socketError ()) + '.';
			break;
		case QXmppClient::KeepAliveError:
			msg = tr ("Keep alive error.");
			break;
		case QXmppClient::XmppStreamError:
			msg = tr ("XMPP error:") + ' ' + QString::number (Client_->xmppStreamError ()) + '.';
			break;
		case QXmppClient::NoError:
			msg = tr ("No error.");
			break;
		}

		ShowMessage (msg);

		SetState (State::Error);
	}

	void RegFormHandlerWidget::handleIqReceived (const QXmppIq& iq)
	{
		if (iq.id () != LastStanzaID_)
			return;

		switch (State_)
		{
		case State::FetchingForm:
			HandleRegForm (iq);
			break;
		case State::AwaitingRegistrationResult:
			HandleRegResult (iq);
			break;
		default:
			qWarning () << Q_FUNC_INFO
					<< "wrong state for incoming iq";
			break;
		}
	}
}
}
}
