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
#include <QFileDialog>
#include <QMessageBox>
#include <interfaces/ianemitter.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <util/resourceloader.h>
#include <util/util.h>
#include "xmlsettingsmanager.h"
#include "matchconfigdialog.h"
#include "typedmatchers.h"
#include "core.h"
#include <QInputDialog>

namespace LeechCraft
{
namespace AdvancedNotifications
{
	const QString CatIM = "org.LC.AdvNotifications.IM";
	const QString TypeIMAttention = "org.LC.AdvNotifications.IM.AttentionDrawn";
	const QString TypeIMIncFile = "org.LC.AdvNotifications.IM.IncomingFile";
	const QString TypeIMIncMsg = "org.LC.AdvNotifications.IM.IncomingMessage";
	const QString TypeIMMUCHighlight = "org.LC.AdvNotifications.IM.MUCHighlightMessage";
	const QString TypeIMMUCInvite = "org.LC.AdvNotifications.IM.MUCInvitation";
	const QString TypeIMMUCMsg = "org.LC.AdvNotifications.IM.MUCMessage";
	const QString TypeIMStatusChange = "org.LC.AdvNotifications.IM.StatusChange";
	const QString TypeIMSubscrGrant = "org.LC.AdvNotifications.IM.Subscr.Granted";
	const QString TypeIMSubscrRevoke = "org.LC.AdvNotifications.IM.Subscr.Revoked";
	const QString TypeIMSubscrRequest = "org.LC.AdvNotifications.IM.Subscr.Requested";
	const QString TypeIMSubscrSub = "org.LC.AdvNotifications.IM.Subscr.Subscribed";
	const QString TypeIMSubscrUnsub = "org.LC.AdvNotifications.IM.Subscr.Unsubscribed";

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
		Type2HR_ [TypeIMMUCInvite] = tr ("MUC invitation");
		Type2HR_ [TypeIMMUCMsg] = tr ("General MUC message");
		Type2HR_ [TypeIMStatusChange] = tr ("Contact status change");
		Type2HR_ [TypeIMSubscrGrant] = tr ("Authorization granted");
		Type2HR_ [TypeIMSubscrRevoke] = tr ("Authorization revoked");
		Type2HR_ [TypeIMSubscrRequest] = tr ("Authorization requested");
		Type2HR_ [TypeIMSubscrSub] = tr ("Contact subscribed");
		Type2HR_ [TypeIMSubscrUnsub] = tr ("Contact unsubscribed");

		Cat2Types_ [CatIM] << TypeIMAttention
				<< TypeIMIncFile
				<< TypeIMIncMsg
				<< TypeIMMUCHighlight
				<< TypeIMMUCInvite
				<< TypeIMMUCMsg
				<< TypeIMStatusChange
				<< TypeIMSubscrGrant
				<< TypeIMSubscrRequest
				<< TypeIMSubscrRevoke
				<< TypeIMSubscrSub
				<< TypeIMSubscrUnsub;

		Ui_.setupUi (this);
		Ui_.RulesTree_->setModel (RulesModel_);
		Ui_.MatchesTree_->setModel (MatchesModel_);

		connect (Ui_.RulesTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex)));

		connect (RulesModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));

		Q_FOREACH (const QString& cat, Cat2HR_.keys ())
			Ui_.EventCat_->addItem (Cat2HR_ [cat], cat);
		on_EventCat__activated (0);

		LoadSettings ();

		XmlSettingsManager::Instance ().RegisterObject ("AudioTheme",
				this, "resetAudioFileBox");
		resetAudioFileBox ();
	}

	QList<NotificationRule> NotificationRulesWidget::GetRules () const
	{
		return Rules_;
	}

	void NotificationRulesWidget::SetRuleEnabled (const NotificationRule& rule, bool enabled)
	{
		const int idx = Rules_.indexOf (rule);
		if (idx == -1)
			return;

		Rules_ [idx].SetEnabled (enabled);
		QStandardItem *item = RulesModel_->item (idx);
		if (item)
			item->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
	}

	void NotificationRulesWidget::LoadDefaultRules ()
	{
		NotificationRule chatMsg (tr ("Incoming chat messages"), CatIM,
				QStringList (TypeIMIncMsg));
		chatMsg.SetMethods (NMVisual | NMTray | NMAudio);
		chatMsg.SetAudioParams (AudioParams ("im-incoming-message"));
		Rules_ << chatMsg;

		NotificationRule mucHigh (tr ("MUC highlights"), CatIM,
				QStringList (TypeIMMUCHighlight));
		mucHigh.SetMethods (NMVisual | NMTray | NMAudio);
		mucHigh.SetAudioParams (AudioParams ("im-muc-highlight"));
		Rules_ << mucHigh;

		NotificationRule mucInv (tr ("MUC invitations"), CatIM,
				QStringList (TypeIMMUCInvite));
		mucInv.SetMethods (NMVisual | NMTray | NMAudio);
		mucInv.SetAudioParams (AudioParams ("im-attention"));
		Rules_ << mucInv;

		NotificationRule incFile (tr ("Incoming file transfers"), CatIM,
				QStringList (TypeIMIncFile));
		incFile.SetMethods (NMVisual | NMTray | NMAudio);
		Rules_ << incFile;

		NotificationRule subscrReq (tr ("Subscription requests"), CatIM,
				QStringList (TypeIMSubscrRequest));
		subscrReq.SetMethods (NMVisual | NMTray | NMAudio);
		subscrReq.SetAudioParams (AudioParams ("im-auth-requested"));
		Rules_ << subscrReq;

		NotificationRule subscrChanges (tr ("Subscription changes"), CatIM,
				QStringList (TypeIMSubscrRevoke)
					<< TypeIMSubscrGrant
					<< TypeIMSubscrSub
					<< TypeIMSubscrUnsub);
		subscrChanges.SetMethods (NMVisual | NMTray);
		Rules_ << subscrChanges;

		NotificationRule attentionDrawn (tr ("Attention requests"), CatIM,
				QStringList (TypeIMAttention));
		attentionDrawn.SetMethods (NMVisual | NMTray | NMAudio);
		attentionDrawn.SetAudioParams (AudioParams ("im-attention"));
		Rules_ << attentionDrawn;
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
		if (Ui_.NotifyCmd_->checkState () == Qt::Checked)
			methods |= NMCommand;
		rule.SetMethods (methods);

		rule.SetFieldMatches (Matches_);

		const int audioIdx = Ui_.AudioFile_->currentIndex ();
		const QString& audioFile = audioIdx >= 0 ?
				Ui_.AudioFile_->itemData (audioIdx).toString () :
				QString ();
		rule.SetAudioParams (AudioParams (audioFile));

		QStringList cmdArgs;
		for (int i = 0; i < Ui_.CommandArgsTree_->topLevelItemCount (); ++i)
			cmdArgs << Ui_.CommandArgsTree_->topLevelItem (i)->text (0);
		rule.SetCmdParams (CmdParams (Ui_.CommandLineEdit_->text ().simplified (), cmdArgs));

		const QModelIndex& curIdx = Ui_.RulesTree_->currentIndex ();
		QStandardItem *item = RulesModel_->itemFromIndex (curIdx.sibling (curIdx.row (), 0));
		rule.SetEnabled (item ? item->checkState () == Qt::Checked : true);

		rule.SetSingleShot (Ui_.RuleSingleShot_->checkState () == Qt::Checked);

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

		items.first ()->setCheckable (true);
		items.first ()->setCheckState (rule.IsEnabled () ? Qt::Checked : Qt::Unchecked);

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
			auto pos = std::find_if (fields.begin (), fields.end (),
					[&fieldName] (decltype (fields.front ()) field) { return field.ID_ == fieldName; });
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
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		settings.setValue ("RulesList", QVariant::fromValue<QList<NotificationRule> > (Rules_));
		settings.endGroup ();
	}

	void NotificationRulesWidget::handleItemSelected (const QModelIndex& index)
	{
		resetAudioFileBox ();
		Ui_.CommandArgsTree_->clear ();
		Ui_.CommandLineEdit_->setText (QString ());

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
		Ui_.NotifyCmd_->setCheckState ((methods & NMCommand) ?
				Qt::Checked : Qt::Unchecked);

		ResetMatchesModel ();
		Matches_ = rule.GetFieldMatches ();
		Q_FOREACH (const FieldMatch& m, Matches_)
			MatchesModel_->appendRow (MatchToRow (m));

		const AudioParams& params = rule.GetAudioParams ();
		if (params.Filename_.isEmpty ())
			Ui_.AudioFile_->setCurrentIndex (-1);
		else
		{
			int idx = Ui_.AudioFile_->findData (params.Filename_);
			if (idx == -1)
				idx = Ui_.AudioFile_->findText (params.Filename_);

			if (idx == -1)
			{
				Ui_.AudioFile_->insertItem (0, params.Filename_, params.Filename_);
				idx = 0;
			}

			Ui_.AudioFile_->setCurrentIndex (idx);
		}

		const CmdParams& cmdParams = rule.GetCmdParams ();
		if (!cmdParams.Cmd_.isEmpty ())
		{
			Ui_.CommandLineEdit_->setText (cmdParams.Cmd_);

			Q_FOREACH (const QString& arg, cmdParams.Args_)
				new QTreeWidgetItem (Ui_.CommandArgsTree_, QStringList (arg));
		}

		Ui_.RuleSingleShot_->setChecked (rule.IsSingleShot () ?
					Qt::Checked :
					Qt::Unchecked);
	}

	void NotificationRulesWidget::handleItemChanged (QStandardItem *item)
	{
		const int idx = item->row ();
		const bool newState = item->checkState () == Qt::Checked;

		if (newState == Rules_.at (idx).IsEnabled () ||
				Rules_.at (idx).IsNull ())
			return;

		Rules_ [idx].SetEnabled (newState);

		SaveSettings ();
	}

	void NotificationRulesWidget::on_AddRule__released ()
	{
		Rules_.prepend (NotificationRule ());
		RulesModel_->insertRow (0, RuleToRow (NotificationRule ()));
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

	void NotificationRulesWidget::on_DefaultRules__released ()
	{
		if (QMessageBox::question (this,
					"LeechCraft",
					tr ("Are you sure you want to replace all rules with "
						"the default set?"),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		Rules_.clear ();
		RulesModel_->clear ();

		LoadDefaultRules ();
		ResetModel ();
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

	void NotificationRulesWidget::on_NotifyVisual__stateChanged (int)
	{
	}

	void NotificationRulesWidget::on_NotifySysTray__stateChanged (int)
	{
	}

	void NotificationRulesWidget::on_NotifyAudio__stateChanged (int state)
	{
		Ui_.PageAudio_->setEnabled (state == Qt::Checked);
	}

	void NotificationRulesWidget::on_NotifyCmd__stateChanged (int state)
	{
		Ui_.PageCommand_->setEnabled (state == Qt::Checked);
	}

	void NotificationRulesWidget::on_BrowseAudioFile__released ()
	{
		const QString& fname = QFileDialog::getOpenFileName (this,
				tr ("Select audio file"),
				QDir::homePath (),
				tr ("Audio files (*.ogg *.wav *.flac *.mp3);;All files (*.*)"));
		if (fname.isEmpty ())
			return;

		const bool shouldReplace = Ui_.AudioFile_->count () &&
				Ui_.AudioFile_->itemText (0) == Ui_.AudioFile_->itemData (0);
		if (shouldReplace)
		{
			Ui_.AudioFile_->setItemText (0, fname);
			Ui_.AudioFile_->setItemData (0, fname);
		}
		else
			Ui_.AudioFile_->insertItem (0, fname, fname);

		Ui_.AudioFile_->setCurrentIndex (0);
	}

	void NotificationRulesWidget::on_TestAudio__released ()
	{
		const int idx = Ui_.AudioFile_->currentIndex ();
		if (idx == -1)
			return;

		const QString& path = Ui_.AudioFile_->itemData (idx).toString ();
		if (path.isEmpty ())
			return;

		const Entity& e = Util::MakeEntity (path, QString (), Internal);
		Core::Instance ().SendEntity (e);
	}

	void NotificationRulesWidget::on_AddArgument__released()
	{
		const QString& text = QInputDialog::getText (this,
				"LeechCraft",
				tr ("Please enter the argument:"));
		if (text.isEmpty ())
			return;

		new QTreeWidgetItem (Ui_.CommandArgsTree_, QStringList (text));
		SaveSettings ();
	}

	void NotificationRulesWidget::on_ModifyArgument__released()
	{
		QTreeWidgetItem *item = Ui_.CommandArgsTree_->currentItem ();
		if (!item)
			return;

		const QString& newText = QInputDialog::getText (this,
				"LeechCraft",
				tr ("Please enter new argument text:"),
				QLineEdit::Normal,
				item->text (0));
		if (newText.isEmpty () ||
				newText == item->text (0))
			return;

		item->setText (0, newText);

		SaveSettings ();
	}

	void NotificationRulesWidget::on_RemoveArgument__released()
	{
		const QModelIndex& index = Ui_.CommandArgsTree_->currentIndex ();
		if (!index.isValid ())
			return;

		delete Ui_.CommandArgsTree_->takeTopLevelItem (index.row ());

		SaveSettings ();
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
