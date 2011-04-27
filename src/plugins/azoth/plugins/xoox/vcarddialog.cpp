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

#include "vcarddialog.h"
#include <QXmppVCardIq.h>
#include "entrybase.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "capsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	VCardDialog::VCardDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.EditBirthday_->setVisible (false);
	}
	
	VCardDialog::VCardDialog (EntryBase *entry, QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.EditBirthday_->setVisible (false);
		
		GlooxAccount *acc = qobject_cast<GlooxAccount*> (entry->GetParentAccount ());
		CapsManager *mgr = acc->GetClientConnection ()->GetCapsManager ();
		
		QString html;
		Q_FOREACH (const QString& variant, entry->Variants ())
		{
			const QMap<QString, QVariant>& info = entry->GetClientInfo (variant);
			const QString& client = info ["client_name"].toString ();
			
			html += "<strong>" + client + "</strong> (" +
					QString::number (info ["priority"].toInt ()) + ")<br />";
					
			const QStringList& caps =
					mgr->GetCaps (entry->GetVariantVerString (variant));
			if (caps.size ())
				html += "<strong>" + tr ("Capabilities") +
						"</strong>:<ul><li>" + caps.join ("</li><li>") + "</li></ul>";
		}
		
		Ui_.ClientInfo_->setHtml (html);
	}

	void VCardDialog::UpdateInfo (const QXmppVCardIq& vcard)
	{
		setWindowTitle (tr ("VCard for %1")
					.arg (vcard.nickName()));

		Ui_.EditRealName_->setText (vcard.fullName ());
		Ui_.EditNick_->setText (vcard.nickName ());
		const QDate& date = vcard.birthday ();
		if (date.isValid ())
			Ui_.EditBirthday_->setDate (date);
		Ui_.EditBirthday_->setVisible (date.isValid ());

		Ui_.EditPhone_->setText ("<phones not supported yet>");

		Ui_.EditURL_->setText (vcard.url ());

		QPixmap px = QPixmap::fromImage (QImage::fromData (vcard.photo ()));
		if (!px.isNull ())
		{
			const QSize& maxPx = Ui_.LabelPhoto_->maximumSize ();
			if (px.width () > maxPx.width () ||
					px.height () > maxPx.height ())
				px = px.scaled (maxPx, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			Ui_.LabelPhoto_->setPixmap (px);
		}
		else
			Ui_.LabelPhoto_->setText (tr ("No photo"));
	}
}
}
}
