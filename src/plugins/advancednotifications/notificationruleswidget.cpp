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
#include <boost/bind.hpp>
#include <QSettings>
#include <QStandardItemModel>
#include <interfaces/ianemitter.h>
#include <util/resourceloader.h>
#include "xmlsettingsmanager.h"
#include "matchconfigdialog.h"
#include "typedmatchers.h"
#include "core.h"

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
	, RulesModel_ (new QStandardItemModel (this))
	, MatchesModel_ (new QStandardItemModel (this))
	{
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
				
		Ui_.setupUi (this);
		Ui_.RulesTree_->setModel (RulesModel_);
		Ui_.MatchesTree_->setModel (MatchesModel_);
		
		connect (Ui_.RulesTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex)));
				
		Q_FOREACH (const QString& cat, Cat2HR_.keys ())
			Ui_.EventCat_->addItem (Cat2HR_ [cat], cat);
		on_EventCat__activated (0);
		
		qRegisterMetaType<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaTypeStreamOperators<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		
		LoadSettings ();
		
		XmlSettingsManager::Instance ().RegisterObject ("AudioTheme",
				this, "resetAudioFileBox");
		resetAudioFileBox ();
	}
	
	QList<NotificationRule> NotificationRulesWidget::GetRules () const
	{
		return Rules_;
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
		RulesModel_->clear ();
		RulesModel_->setHorizontalHeaderLabels (QStringList (tr ("Name"))
				<< tr ("Category")
				<< tr ("Type"));
		
		Q_FOREACH (const NotificationRule& rule, Rules_)
			RulesModel_->appendRow (RuleToRow (rule));
	}
	
	void NotificationRulesWidget::ResetMatchesModel ()
	{
		MatchesModel_->clear ();
		MatchesModel_->setHorizontalHeaderLabels (QStringList (tr ("Field name"))
				<< tr ("Rule description"));
	}
	
	QStringList NotificationRulesWidget::GetSelectedTypes () const
	{
		QStringList types;
		for (int i = 0; i < Ui_.EventTypes_->topLevelItemCount (); ++i)
		{
			QTreeWidgetItem *item = Ui_.EventTypes_->topLevelItem (i);
			if (item->checkState (0) == Qt::Checked)
				types << item->data (0, Qt::UserRole).toString ();
		}
		return types;
	}
	
	NotificationRule NotificationRulesWidget::GetRuleFromUI () const
	{
		const QStringList& types = GetSelectedTypes ();
		
		if (types.isEmpty () ||
				Ui_.RuleName_->text ().isEmpty ())
			return NotificationRule ();
		
		NotificationRule rule (Ui_.RuleName_->text (),
				Ui_.EventCat_->itemData (Ui_.EventCat_->currentIndex ()).toString (),
				types);

		NotificationMethods methods = NMNone;
		if (Ui_.NotifyVisual_->checkState () == Qt::Checked)
			methods |= NMVisual;
		if (Ui_.NotifySysTray_->checkState () == Qt::Checked)
			methods |= NMTray;
		if (Ui_.NotifyAudio_->checkState () == Qt::Checked)
			methods |= NMAudio;
		rule.SetMethods (methods);
		
		rule.SetFieldMatches (Matches_);
		
		const int audioIdx = Ui_.AudioFile_->currentIndex ();
		AudioParams params =
		{
			audioIdx >= 0 ?
				Ui_.AudioFile_->itemData (audioIdx).toString () :
				QString ()
		};
		
		rule.SetAudioParams (params);
		
		return rule;
	}
	
	QList<QStandardItem*> NotificationRulesWidget::RuleToRow (const NotificationRule& rule) const
	{
		QStringList hrTypes;
		Q_FOREACH (const QString& type, rule.GetTypes ())
			hrTypes << Type2HR_ [type];

		QList<QStandardItem*> items;
		items << new QStandardItem (rule.GetName ());
		items << new QStandardItem (Cat2HR_ [rule.GetCategory ()]);
		items << new QStandardItem (hrTypes.join ("; "));
		return items;
	}
	
	namespace
	{
		struct FieldMatchFinder
		{
			const QString& F_;
		
			FieldMatchFinder (const QString& f)
			: F_ (f)
			{
			}
			
			bool operator() (const ANFieldData& fd)
			{
				return fd.ID_ == F_;
			}
		};
	}
	
	QList<QStandardItem*> NotificationRulesWidget::MatchToRow (const FieldMatch& match) const
	{
		QString fieldName = match.GetFieldName ();

		QObject *pObj = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetPluginByID (match.GetPluginID ().toUtf8 ());
		IANEmitter *iae = qobject_cast<IANEmitter*> (pObj);
		if (!iae)
			qWarning () << Q_FUNC_INFO
					<< pObj
					<< "doesn't implement IANEmitter";
		else
		{
			const QList<ANFieldData>& fields = iae->GetANFields ();
			QList<ANFieldData>::const_iterator pos =
					std::find_if (fields.begin (), fields.end (),
							boost::bind (std::equal_to<QString> (),
									fieldName,
									boost::bind (&ANFieldData::ID_,
											_1)));
			if (pos != fields.end ())
				fieldName = pos->Name_;
			else
				qWarning () << Q_FUNC_INFO
						<< "unable to find field"
						<< fieldName;
		}

		QList<QStandardItem*> items;
		items << new QStandardItem (fieldName);
		items << new QStandardItem (match.GetMatcher () ?
				match.GetMatcher ()->GetHRDescription () :
				tr ("<empty matcher>"));
		return items;
	}
	
	void NotificationRulesWidget::SaveSettings () const
	{
		/* Don't save settings until the rest of stuff is working and we
		 * have a sane set of default rules.
		 * 
		 * 
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		settings.setValue ("RulesList", QVariant::fromValue<QList<NotificationRule> > (Rules_));
		settings.endGroup ();
		*/
	}
	
	void NotificationRulesWidget::handleItemSelected (const QModelIndex& index)
	{
		resetAudioFileBox ();

		const NotificationRule& rule = Rules_.value (index.row ());
		
		const int catIdx = Ui_.EventCat_->findData (rule.GetCategory ());
		Ui_.EventCat_->setCurrentIndex (std::max (catIdx, 0));
		
		const QStringList& types = rule.GetTypes ();
		for (int i = 0; i < Ui_.EventTypes_->topLevelItemCount (); ++i)
		{
			QTreeWidgetItem *item = Ui_.EventTypes_->topLevelItem (i);
			const bool cont = types.contains (item->data (0, Qt::UserRole).toString ());
			item->setCheckState (0, cont ? Qt::Checked : Qt::Unchecked);
		}
		
		Ui_.RuleName_->setText (rule.GetName ());

		const NotificationMethods methods = rule.GetMethods ();
		Ui_.NotifyVisual_->setCheckState ((methods & NMVisual) ?
				Qt::Checked : Qt::Unchecked);
		Ui_.NotifySysTray_->setCheckState ((methods & NMTray) ?
				Qt::Checked : Qt::Unchecked);
		Ui_.NotifyAudio_->setCheckState ((methods & NMAudio) ?
				Qt::Checked : Qt::Unchecked);
		
		ResetMatchesModel ();
		Matches_ = rule.GetFieldMatches ();
		Q_FOREACH (const FieldMatch& m, Matches_)
			MatchesModel_->appendRow (MatchToRow (m));
			
		const AudioParams& params = rule.GetAudioParams ();
		const int idx = Ui_.AudioFile_->findData (params.Filename_);
		if (idx == -1 &&
				!params.Filename_.isEmpty ())
			Ui_.AudioFile_->insertItem (0, params.Filename_, params.Filename_);
		else
			Ui_.AudioFile_->setCurrentIndex (idx);
	}
	
	void NotificationRulesWidget::on_AddRule__released ()
	{
		RulesModel_->insertRow (0, RuleToRow (NotificationRule ()));
		Rules_.prepend (NotificationRule ());
	}
	
	void NotificationRulesWidget::on_UpdateRule__released ()
	{
		const QModelIndex& index = Ui_.RulesTree_->currentIndex ();
		if (!index.isValid ())
			return;
		
		const NotificationRule& rule = GetRuleFromUI ();
		if (rule.IsNull ())
			return;
		
		const int row = index.row ();
		Rules_ [row] = rule;
		int i = 0;
		Q_FOREACH (QStandardItem *item, RuleToRow (rule))
			RulesModel_->setItem (row, i++, item);
		
		SaveSettings ();
	}
	
	void NotificationRulesWidget::on_MoveRuleUp__released ()
	{
		const QModelIndex& index = Ui_.RulesTree_->currentIndex ();
		const int row = index.row ();
		if (row < 1)
			return;
		
		std::swap (Rules_ [row - 1], Rules_ [row]);
		RulesModel_->insertRow (row, RulesModel_->takeRow (row - 1));
		
		SaveSettings ();
	}

	void NotificationRulesWidget::on_MoveRuleDown__released ()
	{
		const QModelIndex& index = Ui_.RulesTree_->currentIndex ();
		const int row = index.row () + 1;
		if (row < 0 || row >= RulesModel_->rowCount ())
			return;
		
		std::swap (Rules_ [row - 1], Rules_ [row]);
		RulesModel_->insertRow (row - 1, RulesModel_->takeRow (row));
		
		SaveSettings ();
	}

	void NotificationRulesWidget::on_RemoveRule__released ()
	{
		const QModelIndex& index = Ui_.RulesTree_->currentIndex ();
		if (!index.isValid ())
			return;
		
		RulesModel_->removeRow (index.row ());
		Rules_.removeAt (index.row ());
		
		SaveSettings ();
	}
	
	void NotificationRulesWidget::on_AddMatch__released ()
	{
		MatchConfigDialog dia (GetSelectedTypes (), this);
		if (dia.exec () != QDialog::Accepted)
			return;
		
		const FieldMatch& match = dia.GetFieldMatch ();
		Matches_ << match;
		MatchesModel_->appendRow (MatchToRow (match));
	}
	
	void NotificationRulesWidget::on_ModifyMatch__released ()
	{
		const QModelIndex& index = Ui_.MatchesTree_->currentIndex ();
		if (!index.isValid ())
			return;
		
		MatchConfigDialog dia (GetSelectedTypes (), this);
		if (dia.exec () != QDialog::Accepted)
			return;
		
		const int row = index.row ();
		
		const FieldMatch& match = dia.GetFieldMatch ();
		Matches_ [row] = match;
		int i = 0;
		Q_FOREACH (QStandardItem *item, MatchToRow (match))
			MatchesModel_->setItem (row, i++, item);
	}
	
	void NotificationRulesWidget::on_RemoveMatch__released ()
	{
		const QModelIndex& index = Ui_.MatchesTree_->currentIndex ();
		if (!index.isValid ())
			return;
		
		Matches_.removeAt (index.row ());
		MatchesModel_->removeRow (index.row ());
	}
	
	void NotificationRulesWidget::on_EventCat__activated (int idx)
	{
		const QString& catId = Ui_.EventCat_->itemData (idx).toString ();
		Ui_.EventTypes_->clear ();
		
		Q_FOREACH (const QString& type, Cat2Types_ [catId])
		{
			const QString& hr = Type2HR_ [type];
			QTreeWidgetItem *item = new QTreeWidgetItem (QStringList (hr));
			item->setData (0, Qt::UserRole, type);
			item->setCheckState (0, Qt::Unchecked);
			Ui_.EventTypes_->addTopLevelItem (item);
		}
	}
	
	void NotificationRulesWidget::on_NotifyVisual__stateChanged (int state)
	{
		Ui_.PageVisual_->setEnabled (state == Qt::Checked);
	}
	
	void NotificationRulesWidget::on_NotifySysTray__stateChanged (int state)
	{
		Ui_.PageTray_->setEnabled (state == Qt::Checked);
	}
	
	void NotificationRulesWidget::on_NotifyAudio__stateChanged (int state)
	{
		Ui_.PageAudio_->setEnabled (state == Qt::Checked);
	}
	
	void NotificationRulesWidget::resetAudioFileBox ()
	{
		Ui_.AudioFile_->clear ();
		
		const QString& theme = XmlSettingsManager::Instance ().property ("AudioTheme").toString ();
		const QStringList filters = QStringList ("*.ogg")
				<< "*.wav"
				<< "*.flac"
				<< "*.mp3";
		
		const QFileInfoList& files = Core::Instance ()
				.GetAudioThemeLoader ()->List (theme,
						filters, QDir::Files | QDir::Readable);
		Q_FOREACH (const QFileInfo& file, files)
			Ui_.AudioFile_->addItem (file.baseName (), file.absoluteFilePath ());
	}
}
}
