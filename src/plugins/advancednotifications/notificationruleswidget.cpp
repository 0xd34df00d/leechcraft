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

#include "notificationruleswidget.h"
#include <QSettings>
#include <QStandardItemModel>
#include <interfaces/ianemitter.h>
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace AdvancedNotifications
{
	const QString CatIM = "org.LC.AdvNotifications.IM";
	const QString TypeIMAttention = "org.LC.AdvNotifications.IM.AttentionDrawn";
	const QString TypeIMIncFile = "org.LC.AdvNotifications.IM.IncomingFile";
	const QString TypeIMIncMsg = "org.LC.AdvNotifications.IM.IncomingMessage";
	const QString TypeIMMUCHighlight = "org.LC.AdvNotifications.IM.MUCHighlightMessage";
	const QString TypeIMMUCMsg = "org.LC.AdvNotifications.IM.MUCMessage";
	const QString TypeIMStatusChange = "org.LC.AdvNotifications.IM.StatusChange";
	const QString TypeIMSubscrGrant = "org.LC.AdvNotifications.IM.Subscr.Granted";
	const QString TypeIMSubscrRevoke = "org.LC.AdvNotifications.IM.Subscr.Revoked";
	const QString TypeIMSubscrRequest = "org.LC.AdvNotifications.IM.Subscr.Requested";

	QDataStream& operator<< (QDataStream& out, const NotificationRule& r)
	{
		r.Save (out);
		return out;
	}
	
	QDataStream& operator>> (QDataStream& in, NotificationRule& r)
	{
		r.Load (in);
		return in;
	}

	NotificationRulesWidget::NotificationRulesWidget (QWidget *parent)
	: QWidget (parent)
	, Model_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.RulesTree_->setModel (Model_);
		
		Cat2HR_ [CatIM] = tr ("Instant messaging");
		
		Type2HR_ [TypeIMAttention] = tr ("Attention request");
		Type2HR_ [TypeIMIncFile] = tr ("Incoming file transfer request");
		Type2HR_ [TypeIMIncMsg] = tr ("Incoming chat message");
		Type2HR_ [TypeIMMUCHighlight] = tr ("MUC highlight");
		Type2HR_ [TypeIMMUCMsg] = tr ("General MUC message");
		Type2HR_ [TypeIMStatusChange] = tr ("Contact status change");
		Type2HR_ [TypeIMSubscrGrant] = tr ("Authorization granted");
		Type2HR_ [TypeIMSubscrRevoke] = tr ("Authorization revoked");
		Type2HR_ [TypeIMSubscrRequest] = tr ("Authorization requested");
		
		Cat2Types_ [CatIM] << TypeIMAttention
				<< TypeIMIncFile
				<< TypeIMIncMsg
				<< TypeIMMUCHighlight
				<< TypeIMMUCMsg
				<< TypeIMStatusChange
				<< TypeIMSubscrGrant
				<< TypeIMSubscrRequest
				<< TypeIMSubscrRevoke;
				
		Q_FOREACH (const QString& cat, Cat2HR_.keys ())
			Ui_.EventCat_->addItem (Cat2HR_ [cat], cat);
		on_EventCat__activated (0);
		
		qRegisterMetaType<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaTypeStreamOperators<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		
		LoadSettings ();
	}
	
	void NotificationRulesWidget::LoadDefaultRules ()
	{
		NotificationRule chatMsg (tr ("Incoming chat messages"), CatIM,
				QStringList (TypeIMIncMsg));
		chatMsg.SetMethods (NMVisual | NMTray | NMAudio);
		Rules_ << chatMsg;

		NotificationRule mucHigh (tr ("MUC highlights"), CatIM,
				QStringList (TypeIMMUCHighlight));
		mucHigh.SetMethods (NMVisual | NMTray | NMAudio);
		Rules_ << mucHigh;
		
		NotificationRule incFile (tr ("Incoming file transfers"), CatIM,
				QStringList (TypeIMIncFile));
		incFile.SetMethods (NMVisual | NMTray | NMAudio);
		Rules_ << incFile;
	}
	
	void NotificationRulesWidget::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		Rules_ = settings.value ("RulesList").value<QList<NotificationRule> > ();
		settings.endGroup ();
		
		if (Rules_.isEmpty ())
			LoadDefaultRules ();
		
		ResetModel ();
	}
	
	void NotificationRulesWidget::ResetModel ()
	{
		Model_->clear ();
		Model_->setHorizontalHeaderLabels (QStringList (tr ("Name"))
				<< tr ("Category")
				<< tr ("Type"));
		
		Q_FOREACH (const NotificationRule& rule, Rules_)
		{
			QStringList hrTypes;
			Q_FOREACH (const QString& type, rule.GetTypes ())
				hrTypes << Type2HR_ [type];

			QList<QStandardItem*> items;
			items << new QStandardItem (rule.GetName ());
			items << new QStandardItem (Cat2HR_ [rule.GetCategory ()]);
			items << new QStandardItem (hrTypes.join ("; "));
			Model_->appendRow (items);
		}
	}
	
	void NotificationRulesWidget::SaveSettings () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		settings.setValue ("RulesList", QVariant::fromValue<QList<NotificationRule> > (Rules_));
		settings.endGroup ();
	}
	
	void NotificationRulesWidget::on_EventCat__activated (int idx)
	{
		const QString& catId = Ui_.EventCat_->itemData (idx).toString ();
		Ui_.EventType_->clear ();
		
		Q_FOREACH (const QString& type, Cat2Types_ [catId])
			Ui_.EventType_->addItem (Type2HR_ [type], type);
	}
}
}
