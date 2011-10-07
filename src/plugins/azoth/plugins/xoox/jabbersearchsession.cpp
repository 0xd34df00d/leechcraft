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

#include "jabbersearchsession.h"
#include <QStandardItemModel>
#include "glooxaccount.h"
#include "clientconnection.h"
#include "legacyformbuilder.h"
#include "formbuilder.h"
#include "util.h"
#include <QMessageBox>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	JabberSearchSession::JabberSearchSession (GlooxAccount *acc)
	: QObject (acc)
	, Acc_ (acc)
	, Model_ (new QStandardItemModel (this))
	, SM_ (acc->GetClientConnection ()->GetJabberSearchManager ())
	{
		connect (SM_,
				SIGNAL (gotServerError (QXmppIq)),
				this,
				SLOT (handleGotError (QXmppIq)));
	}

	void JabberSearchSession::RestartSearch (QString server)
	{
		Model_->clear ();

		CurrentServer_ = server;

		connect (SM_,
				SIGNAL (gotSearchFields (QString, QXmppElement)),
				this,
				SLOT (handleGotSearchFields (QString, QXmppElement)),
				Qt::UniqueConnection);
		SM_->RequestSearchFields (server);
	}

	QAbstractItemModel* JabberSearchSession::GetRepresentationModel () const
	{
		return Model_;
	}

	void JabberSearchSession::handleGotItems (const QString& server,
			const QList<JabberSearchManager::Item>& items)
	{
		if (server != CurrentServer_)
			return;

		disconnect (SM_,
				SIGNAL (gotItems (QString, QList<JabberSearchManager::Item>)),
				this,
				SLOT (handleGotItems (QString, QList<JabberSearchManager::Item>)));

		if (items.isEmpty ())
			return;

		const QStringList& keys = items.first ().Dictionary_.keys ();
		Model_->setHorizontalHeaderLabels (keys);

		Q_FOREACH (const JabberSearchManager::Item& item, items)
		{
			QList<QStandardItem*> row;

			Q_FOREACH (const QString& key, keys)
				row << new QStandardItem (item.Dictionary_.value (key, tr ("(unknown)")));

			Model_->appendRow (row);
		}
	}

	void JabberSearchSession::handleGotSearchFields (const QString& server, const QXmppElement& elem)
	{
		if (server != CurrentServer_)
			return;

		disconnect (SM_,
				SIGNAL (gotSearchFields (QString, QXmppElement)),
				this,
				SLOT (handleGotSearchFields (QString, QXmppElement)));

		const QXmppElement& xForm = elem.firstChildElement ("x");
		QXmppDataForm form;
		form.parse (XooxUtil::XmppElem2DomElem (xForm));

		if (!xForm.isNull ())
		{
			FormBuilder fb;
			QWidget *w = fb.CreateForm (form);
			if (!XooxUtil::RunFormDialog (w))
				return;

			QXmppDataForm form = fb.GetForm ();
			form.setType (QXmppDataForm::Submit);

			SM_->SubmitSearchRequest (server, form);
		}
		else
		{
			LegacyFormBuilder fb;
			QWidget *w = fb.CreateForm (elem);
			if (!XooxUtil::RunFormDialog (w))
				return;

			SM_->SubmitSearchRequest (server, fb.GetFilledChildren ());
		}

		connect (SM_,
				SIGNAL (gotItems (QString, QList<JabberSearchManager::Item>)),
				this,
				SLOT (handleGotItems (QString, QList<JabberSearchManager::Item>)),
				Qt::UniqueConnection);
	}

	void JabberSearchSession::handleGotError (const QXmppIq& iq)
	{
		if (CurrentServer_ != iq.from ())
			return;

		QString conditionText;
		switch (iq.error ().condition ())
		{
		case QXmppStanza::Error::ServiceUnavailable:
			conditionText = tr ("search service unavailable");
			break;
		case QXmppStanza::Error::FeatureNotImplemented:
			conditionText = tr ("search feature not implemented");
			break;
		case QXmppStanza::Error::Forbidden:
			conditionText = tr ("search is forbidden");
			break;
		case QXmppStanza::Error::RegistrationRequired:
			conditionText = tr ("registration is required for performing search");
			break;
		case QXmppStanza::Error::NotAllowed:
			conditionText = tr ("search not allowed");
			break;
		case QXmppStanza::Error::NotAuthorized:
			conditionText = tr ("search not authorized");
			break;
		case QXmppStanza::Error::ResourceConstraint:
			conditionText = tr ("too much search requests");
			break;
		default:
			conditionText = tr ("unknown condition %1")
					.arg (iq.error ().condition ());
			break;
		}

		QString text = tr ("Error searching on server %1: %2.")
				.arg (CurrentServer_)
				.arg (conditionText);

		if (!iq.error ().text ().isEmpty ())
			text += " " + tr ("Original error text: %1.")
					.arg (iq.error ().text ());

		QMessageBox::warning (0,
				"Search error",
				text);
	}
}
}
}
