/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "regformhandlerwidget.h"
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QPointer>
#include <QLineEdit>
#include <QDomElement>
#include <QtDebug>
#include "xeps/xmppbobmanager.h"
#include "util.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	RegFormHandlerWidget::RegFormHandlerWidget (QXmppClient *client, QWidget *parent)
	: QWidget (parent)
	, Client_ (client)
	, BobManager_ (client->findExtension<XMPPBobManager> ())
	, FB_ ({}, BobManager_)
	, Widget_ (0)
	, State_ (State::Idle)
	{
		setLayout (new QVBoxLayout);

		connect (Client_,
				SIGNAL (error (QXmppClient::Error)),
				this,
				SLOT (handleError (QXmppClient::Error)));
		connect (Client_,
				SIGNAL (iqReceived (QXmppIq)),
				this,
				SLOT (handleIqReceived (QXmppIq)));
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
			for (const auto edit : Widget_->findChildren<QLineEdit*> ())
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
		queryElem.setAttribute ("xmlns", XooxUtil::NsRegister);

		QXmppIq iq;
		iq.setExtensions ({ queryElem });
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
		queryElem.setAttribute ("xmlns", XooxUtil::NsRegister);

		switch (FormType_)
		{
		case FTLegacy:
			for (const auto& elem : LFB_.GetFilledChildren ())
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
			queryElem.appendChild (dom.documentElement ());
			break;
		}
		}

		QXmppIq iq (QXmppIq::Set);
		if (!ReqJID_.isEmpty ())
			iq.setTo (ReqJID_);
		iq.setExtensions ({ queryElem });
		Client_->sendPacket (iq);
		LastStanzaID_ = iq.id ();

		SetState (State::AwaitingRegistrationResult);
	}

	void RegFormHandlerWidget::Clear ()
	{
		auto widgets = findChildren<QWidget*> ();
		QList<QPointer<QWidget>> pWidgets;
		std::copy (widgets.begin (), widgets.end (), std::back_inserter (pWidgets));
		for (const auto& pWidget : pWidgets)
			delete pWidget;
	}

	void RegFormHandlerWidget::ShowMessage (const QString& msg)
	{
		FB_.Clear ();
		LFB_.Clear ();
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
		if (iq.type () == QXmppIq::Error)
		{
			SetState (State::Error);
			ShowMessage (tr ("Server error: %1.").arg (iq.error ().text ()));
			return;
		}

		QXmppElement queryElem;
		for (const auto& elem : iq.extensions ())
			if (elem.tagName () == "query" &&
					elem.attribute ("xmlns") == XooxUtil::NsRegister)
			{
				queryElem = elem;
				break;
			}

		if (queryElem.isNull ())
		{
			SetState (State::Error);
			ShowMessage (tr ("Service unavailable"));
			return;
		}

		Clear ();

		const auto& formElem = queryElem.firstChildElement ("x");
		if ((formElem.attribute ("xmlns") == XooxUtil::NsRegister ||
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
		for (const auto edit : Widget_->findChildren<QLineEdit*> ())
			connect (edit,
					SIGNAL (textChanged (QString)),
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
		for (const auto& elem : iq.extensions ())
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
