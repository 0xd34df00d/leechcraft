/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "notificationruleswidget.h"
#include <algorithm>
#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <interfaces/an/ianemitter.h>
#include <interfaces/an/constants.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <util/sys/resourceloader.h>
#include <util/xpc/util.h>
#include <util/xpc/stdanfields.h>
#include <util/xpc/anutil.h>
#include <util/sll/containerconversions.h>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include "xmlsettingsmanager.h"
#include "matchconfigdialog.h"
#include "typedmatchers.h"
#include "rulesmanager.h"
#include "audiothememanager.h"
#include "unhandlednotificationskeeper.h"
#include "addfrommisseddialog.h"

namespace LC
{
namespace AdvancedNotifications
{
	NotificationRulesWidget::NotificationRulesWidget (RulesManager *rm,
			const AudioThemeManager *audioMgr,
			const UnhandledNotificationsKeeper *unhandledKeeper,
			QWidget *parent)
	: QWidget (parent)
	, RM_ (rm)
	, AudioThemeManager_ (audioMgr)
	, UnhandledKeeper_ (unhandledKeeper)
	, MatchesModel_ (new QStandardItemModel (this))
	{
		Ui_.setupUi (this);
		Ui_.RulesTree_->setModel (RM_->GetRulesModel ());
		Ui_.MatchesTree_->setModel (MatchesModel_);

		connect (Ui_.RulesTree_->selectionModel (),
				SIGNAL (currentChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (handleItemSelected (QModelIndex, QModelIndex)));

		connect (rm,
				SIGNAL (focusOnRule (QModelIndex)),
				this,
				SLOT (selectRule (QModelIndex)));

		for (const auto& pair : Util::Stlize (Util::AN::GetCategoryNameMap ()))
			Ui_.EventCat_->addItem (pair.second, pair.first);
		on_EventCat__currentIndexChanged (0);

		XmlSettingsManager::Instance ().RegisterObject ("AudioTheme",
				this, "resetAudioFileBox");
		resetAudioFileBox ();
	}

	void NotificationRulesWidget::ResetMatchesModel ()
	{
		MatchesModel_->clear ();
		MatchesModel_->setHorizontalHeaderLabels ({ tr ("Field name"), tr ("Rule description") });
	}

	QString NotificationRulesWidget::GetCurrentCat () const
	{
		const auto idx = Ui_.EventCat_->currentIndex ();
		return Ui_.EventCat_->itemData (idx).toString ();
	}

	QStringList NotificationRulesWidget::GetSelectedTypes () const
	{
		QStringList types;
		for (int i = 0; i < Ui_.EventTypes_->topLevelItemCount (); ++i)
		{
			auto item = Ui_.EventTypes_->topLevelItem (i);
			if (item->checkState (0) == Qt::Checked)
				types << item->data (0, Qt::UserRole).toString ();
		}
		if (types.isEmpty ())
			types = Util::AN::GetKnownEventTypes (GetCurrentCat ());
		return types;
	}

	NotificationRule NotificationRulesWidget::GetRuleFromUI (QModelIndex curIdx) const
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
		if (Ui_.NotifyUrgent_->checkState () == Qt::Checked)
			methods |= NMUrgentHint;
		if (Ui_.NotifySystemDependent_->checkState () == Qt::Checked)
			methods |= NMSystemDependent;
		rule.SetMethods (methods);

		rule.SetFieldMatches (Matches_);

		const int audioIdx = Ui_.AudioFile_->currentIndex ();
		const QString& audioFile = audioIdx >= 0 ?
				Ui_.AudioFile_->itemText (audioIdx) :
				QString ();
		rule.SetAudioParams ({ audioFile });

		QStringList cmdArgs;
		for (int i = 0; i < Ui_.CommandArgsTree_->topLevelItemCount (); ++i)
			cmdArgs << Ui_.CommandArgsTree_->topLevelItem (i)->text (0);
		rule.SetCmdParams ({ .Cmd_ = Ui_.CommandLineEdit_->text ().simplified (), .Args_ = cmdArgs });

		if (!curIdx.isValid ())
			curIdx = Ui_.RulesTree_->currentIndex ();
		rule.SetEnabled (curIdx.sibling (curIdx.row (), 0).data (Qt::CheckStateRole) == Qt::Checked);
		rule.SetSingleShot (Ui_.RuleSingleShot_->checkState () == Qt::Checked);

		rule.SetColor (Ui_.ColorButton_->GetColor ());

		return rule;
	}

	namespace
	{
		QList<ANFieldData> GetPluginFields (const QByteArray& pluginId)
		{
			if (pluginId.isEmpty ())
				return {};

			auto pObj = GetProxyHolder ()->GetPluginsManager ()->GetPluginByID (pluginId);
			if (auto iae = qobject_cast<IANEmitter*> (pObj))
				return iae->GetANFields ();

			return {};
		}
	}

	QList<QStandardItem*> NotificationRulesWidget::MatchToRow (const FieldMatch& match) const
	{
		auto fieldName = match.GetFieldName ();

		const auto& fields = Util::GetStdANFields ({}) + GetPluginFields (match.GetPluginID ().toUtf8 ());

		const auto pos = std::find_if (fields.begin (), fields.end (),
				[&fieldName] (const auto& field) { return field.ID_ == fieldName; });
		if (pos != fields.end ())
			fieldName = pos->Name_;
		else
			qWarning () << Q_FUNC_INFO
					<< "unable to find field"
					<< fieldName;

		QList<QStandardItem*> items;
		items << new QStandardItem (fieldName);
		items << new QStandardItem (match.GetMatcher () ?
				match.GetMatcher ()->GetHRDescription () :
				tr ("<empty matcher>"));
		return items;
	}

	QHash<QObject*, QList<ANFieldData>> NotificationRulesWidget::GetRelevantANFieldsWPlugins () const
	{
		QHash<QObject*, QList<ANFieldData>> result;
		result [nullptr] += Util::GetStdANFields (GetCurrentCat ());
		for (const auto& type : GetSelectedTypes ())
			result [nullptr] += Util::GetStdANFields (type);

		const auto& emitters = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableRoots<IANEmitter*> ();
		for (auto emitterObj : emitters)
		{
			auto emitter = qobject_cast<IANEmitter*> (emitterObj);
			for (const auto& field : emitter->GetANFields ())
				if (!Util::AsSet (GetSelectedTypes ()).intersect (Util::AsSet (field.EventTypes_)).isEmpty ())
					result [emitterObj] << field;
		}

		return result;
	}

	QList<ANFieldData> NotificationRulesWidget::GetRelevantANFields () const
	{
		QList<ANFieldData> result;
		for (const auto& sublist : GetRelevantANFieldsWPlugins ())
			result += sublist;
		return result;
	}

	QString NotificationRulesWidget::GetArgumentText ()
	{
		const auto& fields = GetRelevantANFields ();

		if (fields.isEmpty ())
			return QInputDialog::getText (this,
					"LeechCraft",
					tr ("Please enter the argument:"));

		QStringList items;
		for (const auto& field : fields)
			items << tr ("Custom field %1 (%2)")
					.arg (field.Name_)
					.arg (field.Description_);

		bool ok = false;
		const auto& value = QInputDialog::getItem (this,
				"LeechCraft",
				tr ("Please enter the argument:"),
				items,
				0,
				true,
				&ok);

		if (value.isEmpty () || !ok)
			return {};

		const auto idx = items.indexOf (value);
		return idx >= 0 ?
				('$' + fields.value (idx).ID_) :
				value;
	}

	void NotificationRulesWidget::handleItemSelected (const QModelIndex& index, const QModelIndex& prevIndex)
	{
		if (prevIndex.isValid ())
		{
			const auto& prevRule = RM_->GetRulesList ().value (prevIndex.row ());
			const auto& uiRule = GetRuleFromUI (prevIndex);
			if (uiRule != prevRule &&
					QMessageBox::question (this,
							"LeechCraft",
							tr ("The rule has been changed. Do you want to save it?"),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
				RM_->UpdateRule (prevIndex, uiRule);
		}

		resetAudioFileBox ();
		Ui_.CommandArgsTree_->clear ();
		Ui_.CommandLineEdit_->setText (QString ());

		const auto& rule = RM_->GetRulesList ().value (index.row ());

		const int catIdx = Ui_.EventCat_->findData (rule.GetCategory ());
		Ui_.EventCat_->setCurrentIndex (std::max (catIdx, 0));

		const auto& types = rule.GetTypes ();
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
		Ui_.NotifyUrgent_->setCheckState ((methods & NMUrgentHint) ?
				Qt::Checked : Qt::Unchecked);
		Ui_.NotifySystemDependent_->setCheckState ((methods & NMSystemDependent) ?
				Qt::Checked : Qt::Unchecked);

		ResetMatchesModel ();
		Matches_ = rule.GetFieldMatches ();
		for (const auto& m : Matches_)
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

			for (const auto& arg : cmdParams.Args_)
				new QTreeWidgetItem (Ui_.CommandArgsTree_, QStringList (arg));
		}

		Ui_.RuleSingleShot_->setChecked (rule.IsSingleShot ());

		Ui_.ColorButton_->SetColor (rule.GetColor ());
	}

	void NotificationRulesWidget::selectRule (const QModelIndex& index)
	{
		Ui_.RulesTree_->selectionModel ()->select (index,
				QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
		Ui_.RulesTree_->setCurrentIndex (index);
	}

	void NotificationRulesWidget::on_AddRule__released ()
	{
		RM_->PrependRule ();
	}

	void NotificationRulesWidget::on_AddFromMissed__released ()
	{
		auto dia = new AddFromMissedDialog { UnhandledKeeper_->GetUnhandledModel (), this };
		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->show ();

		auto handleAccepted = [this, dia]
		{
			const auto& idxs = dia->GetSelectedRows ();
			for (const auto& entity : UnhandledKeeper_->GetRulesEntities (idxs))
				if (const auto rule = RM_->CreateRuleFromEntity (entity))
					RM_->PrependRule (*rule);
		};
		connect (dia, &QDialog::accepted, this, handleAccepted);
	}

	void NotificationRulesWidget::on_UpdateRule__released ()
	{
		const auto& index = Ui_.RulesTree_->currentIndex ();
		if (!index.isValid ())
			return;

		RM_->UpdateRule (index, GetRuleFromUI ());
		Ui_.RulesTree_->setCurrentIndex (index);
	}

	void NotificationRulesWidget::on_MoveRuleUp__released ()
	{
		RM_->moveUp (Ui_.RulesTree_->currentIndex ());
	}

	void NotificationRulesWidget::on_MoveRuleDown__released ()
	{
		RM_->moveDown (Ui_.RulesTree_->currentIndex ());
	}

	void NotificationRulesWidget::on_RemoveRule__released ()
	{
		const QModelIndex& index = Ui_.RulesTree_->currentIndex ();
		if (!index.isValid ())
			return;

		RM_->removeRule (index);
	}

	void NotificationRulesWidget::on_DefaultRules__released ()
	{
		if (QMessageBox::question (this,
					"LeechCraft",
					tr ("Are you sure you want to replace all rules with "
						"the default set?"),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		RM_->reset ();
	}

	void NotificationRulesWidget::on_AddMatch__released ()
	{
		MatchConfigDialog dia (GetRelevantANFieldsWPlugins (), this);
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

		const int row = index.row ();

		MatchConfigDialog dia (GetRelevantANFieldsWPlugins (), this);
		dia.SetFieldMatch (Matches_.value (row));
		if (dia.exec () != QDialog::Accepted)
			return;

		const FieldMatch& match = dia.GetFieldMatch ();
		Matches_ [row] = match;
		int i = 0;
		for (QStandardItem *item : MatchToRow (match))
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

	void NotificationRulesWidget::on_EventCat__currentIndexChanged (int)
	{
		const auto& catId = GetCurrentCat ();
		Ui_.EventTypes_->clear ();

		for (const auto& type : Util::AN::GetKnownEventTypes (catId))
		{
			const auto& hr = Util::AN::GetTypeName (type);
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

		const auto& e = Util::MakeEntity (path, {}, Internal | FromUserInitiated);
		const bool wasHandled = GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
		if (!wasHandled)
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("No plugin has been found to play %1.")
						.arg ("<em>" + path + "</em>"));
	}

	void NotificationRulesWidget::on_AddArgument__released()
	{
		const auto& text = GetArgumentText ();
		if (text.isEmpty ())
			return;

		new QTreeWidgetItem (Ui_.CommandArgsTree_, QStringList (text));
	}

	void NotificationRulesWidget::on_ModifyArgument__released()
	{
		QTreeWidgetItem *item = Ui_.CommandArgsTree_->currentItem ();
		if (!item)
			return;

		const auto& text = GetArgumentText ();
		if (text.isEmpty ())
			return;

		item->setText (0, text);
	}

	void NotificationRulesWidget::on_RemoveArgument__released()
	{
		const QModelIndex& index = Ui_.CommandArgsTree_->currentIndex ();
		if (!index.isValid ())
			return;

		delete Ui_.CommandArgsTree_->takeTopLevelItem (index.row ());
	}

	void NotificationRulesWidget::resetAudioFileBox ()
	{
		Ui_.AudioFile_->clear ();

		const auto& theme = XmlSettingsManager::Instance ().property ("AudioTheme").toString ();
		for (const auto& file : AudioThemeManager_->GetFilesList (theme))
			Ui_.AudioFile_->addItem (file.baseName (), file.absoluteFilePath ());
	}
}
}
