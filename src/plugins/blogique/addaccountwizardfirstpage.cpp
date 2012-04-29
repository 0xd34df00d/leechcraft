/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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
#include <QtDebug>
#include "interfaces/blogique/ibloggingplatform.h"
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
	AddAccountWizardFirstPage::AddAccountWizardFirstPage (QWidget *parent)
	: QWizardPage (parent)
	{
		Ui_.setupUi (this);

		connect (Ui_.ProtoBox_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (readdWidgets ()));
		connect (Ui_.RegisterAccount_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (readdWidgets ()));
	}
	
	void AddAccountWizardFirstPage::initializePage ()
	{
		registerField ("AccountName*", Ui_.NameEdit_);
		registerField ("AccountProto", Ui_.ProtoBox_);
		registerField ("RegisterNewAccount", Ui_.RegisterAccount_);

		const QList<IBloggingPlatform*>& platforms = Core::Instance ().GetBloggingPlatforms ();
		Q_FOREACH (IBloggingPlatform *platform, platforms)
		{
			if (platform->GetFeatures () & IBloggingPlatform::BPFNoAccountRegistration)
				continue;

			Ui_.ProtoBox_->addItem (platform->GetBloggingPlatformIcon (),
					platform->GetBloggingPlatformName (),
					QVariant::fromValue<QObject*> (platform->GetObject ()));
		}
			
		connect (wizard (),
				SIGNAL (accepted ()),
				this,
				SLOT (handleAccepted ()));
	}
	
	void AddAccountWizardFirstPage::readdWidgets ()
	{
		const int idx = Ui_.ProtoBox_->currentIndex ();
		if (idx == -1)
			return;

		QObject *obj = Ui_.ProtoBox_->itemData (idx).value<QObject*> ();
		IBloggingPlatform *platform = qobject_cast<IBloggingPlatform*> (obj);
		if (!platform)
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< obj
					<< "to IBloggingPlatform";
			return;
		}

		Ui_.RegisterAccount_->setEnabled (!(platform->GetFeatures () & IBloggingPlatform::BPFNoAccountRegistration));

		const int currentId = wizard ()->currentId ();
		Q_FOREACH (const int id, wizard ()->pageIds ())
			if (id > currentId)
				wizard ()->removePage (id);
		qDeleteAll (Widgets_);

		Widgets_ = platform->GetAccountRegistrationWidgets ();
		if (!Widgets_.size ())
			return;

		const QString& platformName = platform->GetBloggingPlatformName ();
		Q_FOREACH (QWidget *widget, Widgets_)
		{
			QWizardPage *page = qobject_cast<QWizardPage*> (widget);
			if (!page)
			{
				page = new QWizardPage (wizard ());
				page->setTitle (tr ("%1 options")
						.arg (platformName));
				page->setLayout (new QVBoxLayout ());
				page->layout ()->addWidget (widget);
			}
			wizard ()->addPage (page);
		}
		
		setFinalPage (false);
	}
	
	void AddAccountWizardFirstPage::handleAccepted ()
	{
	}
}
}
