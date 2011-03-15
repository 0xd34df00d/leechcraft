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

#include "addaccountwizardfirstpage.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	AddAccountWizardFirstPage::AddAccountWizardFirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);
	}
	
	void AddAccountWizardFirstPage::initializePage ()
	{
		registerField ("AccountName*", Ui_.NameEdit_);
		registerField ("AccountProto", Ui_.ProtoBox_);

		const QList<IProtocol*>& protos = Core::Instance ().GetProtocols ();
		Q_FOREACH (IProtocol *proto, protos)
			Ui_.ProtoBox_->addItem (proto->GetProtocolName (),
					QVariant::fromValue<QObject*> (proto->GetObject ()));
			
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));
	}
	
	void AddAccountWizardFirstPage::on_ProtoBox__currentIndexChanged (int idx)
	{
		if (idx == -1)
			return;

		QObject *obj = Ui_.ProtoBox_->itemData (idx).value<QObject*> ();
		IProtocol *proto = qobject_cast<IProtocol*> (obj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IProtocol";
			return;
		}
		
		const int currentId = wizard ()->currentId ();
		Q_FOREACH (const int id, wizard ()->pageIds ())
			if (id > currentId)
				wizard ()->removePage (id);
		qDeleteAll (Widgets_);

		Widgets_ = proto->GetAccountRegistrationWidgets ();
		if (!Widgets_.size ())
			return;
		
		const QString& protoName = proto->GetProtocolName ();
		Q_FOREACH (QWidget *widget, Widgets_)
		{
			QWizardPage *page = new QWizardPage (wizard ());
			page->setTitle (tr ("%1 options")
					.arg (protoName));
			page->setLayout (new QVBoxLayout ());
			page->layout ()->addWidget (widget);
			wizard ()->addPage (page);
		}
		
		setFinalPage (false);
	}
	
	void AddAccountWizardFirstPage::handleAccepted ()
	{
		QObject *obj = Ui_.ProtoBox_->itemData (field ("AccountProto").toInt ()).value<QObject*> ();
		IProtocol *proto = qobject_cast<IProtocol*> (obj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IProtocol";
			return;
		}
		
		proto->RegisterAccount (Ui_.NameEdit_->text (), Widgets_);
	}
}
}
