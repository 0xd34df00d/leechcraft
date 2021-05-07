/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rulesmanager.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/an/constants.h>
#include <interfaces/an/ianemitter.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <util/sll/containerconversions.h>
#include <util/sll/prelude.h>
#include <util/xpc/stdanfields.h>
#include <util/xpc/util.h>
#include <util/xpc/anutil.h>
#include <util/models/rolenamesmixin.h>
#include "typedmatchers.h"
#include "xmlsettingsmanager.h"
#include "fields.h"

namespace LC
{
namespace AdvancedNotifications
{
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

	namespace
	{
		class RulesModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Roles
			{
				RuleName = Qt::UserRole + 1,
				IsRuleEnabled
			};

			RulesModel (QObject *parent)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::RuleName] = "ruleName";
				roleNames [Roles::IsRuleEnabled] = "isRuleEnabled";
				setRoleNames (roleNames);
			}
		};
	}

	RulesManager::RulesManager (const ICoreProxy_ptr& proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	, RulesModel_ (new RulesModel (this))
	{
		qRegisterMetaType<NotificationRule> ("LC::AdvancedNotifications::NotificationRule");
		qRegisterMetaTypeStreamOperators<NotificationRule> ("LC::AdvancedNotifications::NotificationRule");
		qRegisterMetaType<QList<NotificationRule>> ("QList<LC::AdvancedNotifications::NotificationRule>");
		qRegisterMetaTypeStreamOperators<QList<NotificationRule>> ("QList<LC::AdvancedNotifications::NotificationRule>");

		LoadSettings ();

		connect (RulesModel_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged (QStandardItem*)));
	}

	QAbstractItemModel* RulesManager::GetRulesModel () const
	{
		return RulesModel_;
	}

	QList<NotificationRule> RulesManager::GetRulesList () const
	{
		return Rules_;
	}

	QList<NotificationRule> RulesManager::GetRules (const Entity& e)
	{
		const QString& type = e.Additional_ [Fields::EventType].toString ();

		QList<NotificationRule> result;

		for (const auto& rule : GetRulesList ())
		{
			if (!rule.IsEnabled ())
				continue;

			if (!rule.GetTypes ().contains (type))
				continue;

			bool fieldsMatch = true;
			for (const auto& match : rule.GetFieldMatches ())
			{
				const QString& fieldName = match.GetFieldName ();
				const auto& matcher = match.GetMatcher ();
				if (!matcher->Match (e.Additional_ [fieldName]))
				{
					fieldsMatch = false;
					break;
				}
			}

			if (!fieldsMatch)
				continue;

			if (rule.IsSingleShot ())
				SetRuleEnabled (rule, false);

			result << rule;
		}

		return result;
	}

	void RulesManager::SetRuleEnabled (const NotificationRule& rule, bool enabled)
	{
		const int idx = Rules_.indexOf (rule);
		if (idx == -1)
			return;

		setRuleEnabled (idx, enabled);
	}

	void RulesManager::UpdateRule (const QModelIndex& index, const NotificationRule& rule)
	{
		if (rule.IsNull ())
			return;

		const int row = index.row ();
		Rules_ [row] = rule;
		int i = 0;
		for (auto item : RuleToRow (rule))
			RulesModel_->setItem (row, i++, item);

		SaveSettings ();
	}

	std::optional<NotificationRule> RulesManager::CreateRuleFromEntity (const Entity& e)
	{
		const auto& title = e.Entity_.toString ();
		const auto& sender = e.Additional_ [Fields::SenderID].toByteArray ();
		const auto& category = e.Additional_ [Fields::EventCategory].toString ();
		const auto& types = e.Additional_ [Fields::EventType].toStringList ();

		const auto plugin = Proxy_->GetPluginsManager ()->GetPluginByID (sender);
		if (!plugin)
		{
			qWarning () << Q_FUNC_INFO
					<< "no plugin for"
					<< sender;
			return {};
		}

		NotificationRule rule (title, category, types);

		if (e.Additional_ ["org.LC.AdvNotifications.SingleShot"].toBool ())
			rule.SetSingleShot (true);

		if (e.Additional_ ["org.LC.AdvNotifications.NotifyPersistent"].toBool ())
			rule.AddMethod (NMTray);

		if (e.Additional_ ["org.LC.AdvNotifications.NotifyTransient"].toBool ())
			rule.AddMethod (NMVisual);

		const auto& typeSet = Util::AsSet (types);
		if (e.Additional_ ["org.LC.AdvNotifications.NotifyAudio"].toBool ())
		{
			rule.AddMethod (NMAudio);

			for (const auto& other : Rules_)
			{
				const auto& audioParams = other.GetAudioParams ();
				if (!audioParams.Filename_.isEmpty () &&
						!other.GetTypes ().intersect (typeSet).isEmpty ())
					rule.SetAudioParams (audioParams);
			}
		}

		auto stdFieldData = Util::GetStdANFields (category);
		for (const auto& type : types)
			stdFieldData += Util::GetStdANFields (type);

		auto tryAddFieldMatch = [&e, &rule, sender] (const ANFieldData& field, bool standard)
		{
			if (!e.Additional_.contains (field.ID_))
				return;

			const auto& valMatcher = TypedMatcherBase::Create (field.Type_);

			const auto& fieldValVar = e.Additional_ [field.ID_];
			if (fieldValVar.canConvert<ANFieldValue> ())
				valMatcher->SetValue (e.Additional_ [field.ID_].value<ANFieldValue> ());
			else
				valMatcher->SetValue (e.Additional_ [field.ID_]);

			FieldMatch fieldMatch (field.Type_, valMatcher);
			fieldMatch.SetPluginID (standard ? QString {} : sender);
			fieldMatch.SetFieldName (field.ID_);
			rule.AddFieldMatch (fieldMatch);
		};
		for (const auto& field : stdFieldData)
			tryAddFieldMatch (field, true);

		if (auto iane = qobject_cast<IANEmitter*> (plugin))
			for (const auto& field : iane->GetANFields ())
			{
				qDebug () << field.ID_ << "testing" << field.EventTypes_ << "against" << typeSet;
				if (!Util::AsSet (field.EventTypes_).intersect (typeSet).isEmpty ())
					tryAddFieldMatch (field, false);
			}

		return rule;
	}

	void RulesManager::HandleEntity (const Entity& e)
	{
		const auto rule = CreateRuleFromEntity (e);
		if (!rule)
			return;

		PrependRule (*rule);

		SaveSettings ();

		if (e.Additional_.value (AN::EF::OpenConfiguration).toBool ())
		{
			XmlSettingsManager::Instance ().ShowSettingsPage ("RulesWidget");
			emit focusOnRule (RulesModel_->index (0, 0));
		}
	}

	void RulesManager::SuggestRuleConfiguration (const Entity& rule)
	{
		XmlSettingsManager::Instance ().ShowSettingsPage ("RulesWidget");

		const auto id = rule.Additional_ ["org.LC.AdvNotifications.RuleID"].toInt ();
		emit focusOnRule (RulesModel_->index (id, 0));
	}

	QList<Entity> RulesManager::GetAllRules (const QString& category) const
	{
		QList<Entity> result;
		for (int i = 0; i < Rules_.size (); ++i)
		{
			const auto& rule = Rules_.at (i);
			if (rule.GetCategory () != category)
				continue;

			auto e = Util::MakeEntity (rule.GetName (), {}, {}, {});
			e.Additional_ ["org.LC.AdvNotifications.RuleID"] = i;
			e.Additional_ [Fields::SenderID] = "org.LeechCraft.AdvancedNotifications";
			e.Additional_ [Fields::EventCategory] = rule.GetCategory ();
			e.Additional_ [Fields::EventType] = QStringList { rule.GetTypes ().values () };
			e.Additional_ ["org.LC.AdvNotifications.AssocColor"] = rule.GetColor ();
			e.Additional_ ["org.LC.AdvNotifications.IsEnabled"] = rule.IsEnabled ();

			for (const auto& fieldMatch : rule.GetFieldMatches ())
			{
				const auto& matcher = fieldMatch.GetMatcher ();
				e.Additional_ [fieldMatch.GetFieldName ()] = QVariant::fromValue (matcher->GetValue ());
			}

			result << e;
		}
		return result;
	}

	void RulesManager::PrependRule (const NotificationRule& rule)
	{
		Rules_.prepend (rule);
		RulesModel_->insertRow (0, RuleToRow (rule));
	}

	void RulesManager::LoadDefaultRules (int version)
	{
		if (version <= 0)
		{
			NotificationRule chatMsg (tr ("Incoming chat messages"), AN::CatIM,
					QStringList (AN::TypeIMIncMsg));
			chatMsg.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint | NMSystemDependent);
			chatMsg.SetAudioParams (AudioParams ("im-incoming-message"));
			Rules_ << chatMsg;

			NotificationRule mucHigh (tr ("MUC highlights"), AN::CatIM,
					QStringList (AN::TypeIMMUCHighlight));
			mucHigh.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint | NMSystemDependent);
			mucHigh.SetAudioParams (AudioParams ("im-muc-highlight"));
			Rules_ << mucHigh;

			NotificationRule mucInv (tr ("MUC invitations"), AN::CatIM,
					QStringList (AN::TypeIMMUCInvite));
			mucInv.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint | NMSystemDependent);
			mucInv.SetAudioParams (AudioParams ("im-attention"));
			Rules_ << mucInv;

			NotificationRule incFile (tr ("Incoming file transfers"), AN::CatIM,
					QStringList (AN::TypeIMIncFile));
			incFile.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint | NMSystemDependent);
			Rules_ << incFile;

			NotificationRule subscrReq (tr ("Subscription requests"), AN::CatIM,
					QStringList (AN::TypeIMSubscrRequest));
			subscrReq.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint | NMSystemDependent);
			subscrReq.SetAudioParams (AudioParams ("im-auth-requested"));
			Rules_ << subscrReq;

			NotificationRule subscrChanges (tr ("Subscription changes"), AN::CatIM,
					QStringList (AN::TypeIMSubscrRevoke)
						<< AN::TypeIMSubscrGrant
						<< AN::TypeIMSubscrSub
						<< AN::TypeIMSubscrUnsub);
			subscrChanges.SetMethods (NMVisual | NMTray);
			Rules_ << subscrChanges;

			NotificationRule attentionDrawn (tr ("Attention requests"), AN::CatIM,
					QStringList (AN::TypeIMAttention));
			attentionDrawn.SetMethods (NMVisual | NMTray | NMAudio | NMUrgentHint | NMSystemDependent);
			attentionDrawn.SetAudioParams (AudioParams ("im-attention"));
			Rules_ << attentionDrawn;
		}

		if (version == -1 || version == 1)
		{
			NotificationRule eventDue (tr ("Event is due"), AN::CatOrganizer,
					QStringList (AN::TypeOrganizerEventDue));
			eventDue.SetMethods (NMVisual | NMTray | NMAudio | NMSystemDependent);
			eventDue.SetAudioParams (AudioParams ("org-event-due"));
			Rules_ << eventDue;
		}

		if (version == -1 || version == 2)
		{
			NotificationRule downloadFinished (tr ("Download finished"), AN::CatDownloads,
					QStringList (AN::TypeDownloadFinished));
			downloadFinished.SetMethods (NMVisual | NMTray | NMAudio | NMSystemDependent);
			Rules_ << downloadFinished;

			NotificationRule downloadError (tr ("Download error"), AN::CatDownloads,
					QStringList (AN::TypeDownloadError));
			downloadError.SetMethods (NMVisual | NMTray | NMAudio | NMSystemDependent);
			downloadError.SetAudioParams (AudioParams ("error"));
			Rules_ << downloadError;
		}

		if (version == -1 || version == 3)
		{
			NotificationRule generic (tr ("Generic"), AN::CatGeneric,
					QStringList (AN::TypeGeneric));
			generic.SetMethods (NMVisual | NMTray | NMSystemDependent);
			Rules_ << generic;
		}

		if (version == -1 || version == 4)
		{
			NotificationRule packageUpdated (tr ("Package updated"), AN::CatPackageManager,
					{ AN::TypePackageUpdated });
			packageUpdated.SetMethods (NMVisual | NMTray | NMSystemDependent);
			Rules_ << packageUpdated;
		}

		if (version == -1 || version == 5)
		{
			FieldMatch match (QVariant::Bool);
			match.SetFieldName (AN::Field::TerminalActive);
			match.GetMatcher ()->SetValue (ANBoolFieldValue { false });

			NotificationRule inactiveBell (tr ("Bell in inactive terminal"), AN::CatTerminal,
					{ AN::TypeTerminalBell });
			inactiveBell.AddFieldMatch (match);
			inactiveBell.SetMethods (NMVisual | NMTray);
			Rules_ << inactiveBell;

			NotificationRule inactiveActivity (tr ("Activity in inactive terminal"), AN::CatTerminal,
					{ AN::TypeTerminalActivity });
			inactiveActivity.AddFieldMatch (match);
			inactiveActivity.SetMethods (NMVisual | NMTray);
			Rules_ << inactiveActivity;

			NotificationRule inactiveInactivity (tr ("Inactivity in inactive terminal"), AN::CatTerminal,
					{ AN::TypeTerminalInactivity });
			inactiveInactivity.AddFieldMatch (match);
			inactiveInactivity.SetMethods (NMVisual | NMTray);
			Rules_ << inactiveInactivity;
		}

		if (version == -1 || version == 6)
		{
			NotificationRule newsSourceUpdated (tr ("News source updated"), AN::CatNews, { AN::TypeNewsSourceUpdated });
			newsSourceUpdated.SetMethods (NMVisual);
			Rules_ << newsSourceUpdated;

			NotificationRule newsSourceBroken (tr ("News source broken"), AN::CatNews, { AN::TypeNewsSourceBroken });
			newsSourceBroken.SetMethods (NMVisual | NMTray);
			Rules_ << newsSourceBroken;
		}
	}

	void RulesManager::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		Rules_ = settings.value ("RulesList").value<QList<NotificationRule>> ();
		int rulesVersion = settings.value ("DefaultRulesVersion", 1).toInt ();

		const int currentDefVersion = 7;
		if (Rules_.isEmpty ())
			LoadDefaultRules (0);

		const bool shouldSave = rulesVersion < currentDefVersion;
		while (rulesVersion < currentDefVersion)
			LoadDefaultRules (rulesVersion++);
		if (shouldSave)
			SaveSettings ();

		settings.setValue ("DefaultRulesVersion", currentDefVersion);
		settings.endGroup ();

		ResetModel ();
	}

	void RulesManager::ResetModel ()
	{
		RulesModel_->clear ();
		RulesModel_->setHorizontalHeaderLabels (QStringList (tr ("Name"))
				<< tr ("Category")
				<< tr ("Type"));

		for (const auto& rule : Rules_)
			RulesModel_->appendRow (RuleToRow (rule));
	}

	void RulesManager::SaveSettings () const
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		settings.setValue ("RulesList", QVariant::fromValue<QList<NotificationRule>> (Rules_));
		settings.endGroup ();

		emit rulesChanged ();
	}

	QList<QStandardItem*> RulesManager::RuleToRow (const NotificationRule& rule) const
	{
		const QStringList hrTypes { Util::Map (rule.GetTypes ().values (), Util::AN::GetTypeName) };

		QList<QStandardItem*> items;
		items << new QStandardItem (rule.GetName ());
		items << new QStandardItem (Util::AN::GetCategoryName (rule.GetCategory ()));
		items << new QStandardItem (hrTypes.join ("; "));

		items.first ()->setCheckable (true);
		items.first ()->setCheckState (rule.IsEnabled () ? Qt::Checked : Qt::Unchecked);

		for (auto item : items)
		{
			item->setData (rule.GetName (), RulesModel::Roles::RuleName);
			item->setData (rule.IsEnabled (), RulesModel::Roles::IsRuleEnabled);
		}

		return items;
	}

	void RulesManager::removeRule (const QModelIndex& index)
	{
		RulesModel_->removeRow (index.row ());
		Rules_.removeAt (index.row ());

		SaveSettings ();
	}

	void RulesManager::moveUp (const QModelIndex& index)
	{
		const int row = index.row ();
		if (row < 1)
			return;

		std::swap (Rules_ [row - 1], Rules_ [row]);
		RulesModel_->insertRow (row, RulesModel_->takeRow (row - 1));

		SaveSettings ();
	}

	void RulesManager::moveDown (const QModelIndex& index)
	{
		const int row = index.row () + 1;
		if (row < 0 || row >= RulesModel_->rowCount ())
			return;

		std::swap (Rules_ [row - 1], Rules_ [row]);
		RulesModel_->insertRow (row - 1, RulesModel_->takeRow (row));

		SaveSettings ();
	}

	void RulesManager::setRuleEnabled (int idx, bool enabled)
	{
		if (Rules_ [idx].IsEnabled () == enabled)
			return;

		Rules_ [idx].SetEnabled (enabled);
		SaveSettings ();

		if (auto item = RulesModel_->item (idx))
		{
			disconnect (RulesModel_,
					SIGNAL (itemChanged (QStandardItem*)),
					this,
					SLOT (handleItemChanged (QStandardItem*)));
			item->setData (enabled, RulesModel::Roles::IsRuleEnabled);
			item->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
			connect (RulesModel_,
					SIGNAL (itemChanged (QStandardItem*)),
					this,
					SLOT (handleItemChanged (QStandardItem*)));
		}
	}

	void RulesManager::reset ()
	{
		Rules_.clear ();
		RulesModel_->clear ();

		LoadDefaultRules ();
		ResetModel ();
		SaveSettings ();
	}

	QVariant RulesManager::getRulesModel () const
	{
		return QVariant::fromValue<QObject*> (RulesModel_);
	}

	void RulesManager::handleItemChanged (QStandardItem *item)
	{
		if (item->column ())
			return;

		const int idx = item->row ();
		const bool newState = item->checkState () == Qt::Checked;

		item->setData (newState, RulesModel::Roles::IsRuleEnabled);

		if (newState == Rules_.at (idx).IsEnabled () ||
				Rules_.at (idx).IsNull ())
			return;

		Rules_ [idx].SetEnabled (newState);

		SaveSettings ();
	}
}
}
