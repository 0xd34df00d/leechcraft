/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 **********************************************************************/

#include "rulesmanager.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QCoreApplication>
#include <QtDebug>
#include <interfaces/an/constants.h>
#include <interfaces/an/ianemitter.h>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <util/xpc/stdanfields.h>
#include <util/xpc/util.h>
#include "core.h"
#include "typedmatchers.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
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
		class RulesModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				RuleName = Qt::UserRole + 1,
				IsRuleEnabled
			};

			RulesModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::RuleName] = "ruleName";
				roleNames [Roles::IsRuleEnabled] = "isRuleEnabled";
				setRoleNames (roleNames);
			}
		};
	}

	RulesManager::RulesManager (QObject *parent)
	: QObject (parent)
	, RulesModel_ (new RulesModel (this))
	{
		qRegisterMetaType<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaTypeStreamOperators<NotificationRule> ("LeechCraft::AdvancedNotifications::NotificationRule");
		qRegisterMetaType<QList<NotificationRule>> ("QList<LeechCraft::AdvancedNotifications::NotificationRule>");
		qRegisterMetaTypeStreamOperators<QList<NotificationRule>> ("QList<LeechCraft::AdvancedNotifications::NotificationRule>");

		Cat2HR_ [AN::CatIM] = tr ("Instant messaging");
		Type2HR_ [AN::TypeIMAttention] = tr ("Attention request");
		Type2HR_ [AN::TypeIMIncFile] = tr ("Incoming file transfer request");
		Type2HR_ [AN::TypeIMIncMsg] = tr ("Incoming chat message");
		Type2HR_ [AN::TypeIMMUCHighlight] = tr ("MUC highlight");
		Type2HR_ [AN::TypeIMMUCInvite] = tr ("MUC invitation");
		Type2HR_ [AN::TypeIMMUCMsg] = tr ("General MUC message");
		Type2HR_ [AN::TypeIMStatusChange] = tr ("Contact status change");
		Type2HR_ [AN::TypeIMSubscrGrant] = tr ("Authorization granted");
		Type2HR_ [AN::TypeIMSubscrRevoke] = tr ("Authorization revoked");
		Type2HR_ [AN::TypeIMSubscrRequest] = tr ("Authorization requested");
		Type2HR_ [AN::TypeIMSubscrSub] = tr ("Contact subscribed");
		Type2HR_ [AN::TypeIMSubscrUnsub] = tr ("Contact unsubscribed");

		Cat2HR_ [AN::CatOrganizer] = tr ("Organizer");
		Type2HR_ [AN::TypeOrganizerEventDue] = tr ("Event is due");

		Cat2HR_ [AN::CatDownloads] = tr ("Downloads");
		Type2HR_ [AN::TypeDownloadError] = tr ("Download error");
		Type2HR_ [AN::TypeDownloadFinished] = tr ("Download finished");

		Cat2HR_ [AN::CatPackageManager] = tr ("Package manager");
		Type2HR_ [AN::TypePackageUpdated] = tr ("Package updated");

		Cat2HR_ [AN::CatMediaPlayer] = tr ("Media player");
		Type2HR_ [AN::TypeMediaPlaybackStatus] = tr ("Media playback status changed");

		Cat2HR_ [AN::CatGeneric] = tr ("Generic");
		Type2HR_ [AN::TypeGeneric] = tr ("Generic");

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

	const QMap<QString, QString>& RulesManager::GetCategory2HR () const
	{
		return Cat2HR_;
	}

	const QMap<QString, QString>& RulesManager::GetType2HR () const
	{
		return Type2HR_;
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

	void RulesManager::HandleEntity (const Entity& e)
	{
		const auto& title = e.Entity_.toString ();
		const auto& sender = e.Additional_ ["org.LC.AdvNotifications.SenderID"].toByteArray ();
		const auto& category = e.Additional_ ["org.LC.AdvNotifications.EventCategory"].toString ();
		const auto& types = e.Additional_ ["org.LC.AdvNotifications.EventType"].toStringList ();

		const auto& proxy = Core::Instance ().GetProxy ();
		const auto plugin = proxy->GetPluginsManager ()->GetPluginByID (sender);
		if (!plugin)
		{
			qWarning () << Q_FUNC_INFO
					<< "no plugin for"
					<< sender;
			return;
		}

		NotificationRule rule (title, category, types);

		if (e.Additional_ ["org.LC.AdvNotifications.SingleShot"].toBool ())
			rule.SetSingleShot (true);

		if (e.Additional_ ["org.LC.AdvNotifications.NotifyPersistent"].toBool ())
			rule.AddMethod (NMTray);

		if (e.Additional_ ["org.LC.AdvNotifications.NotifyTransient"].toBool ())
			rule.AddMethod (NMVisual);

		const auto& typeSet = types.toSet ();
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

		if (auto iane = qobject_cast<IANEmitter*> (plugin))
			for (const auto& field : iane->GetANFields ())
			{
				qDebug () << "testing" << field.EventTypes_ << "against" << typeSet;
				if (!field.EventTypes_.toSet ().intersect (typeSet).isEmpty ())
					stdFieldData << field;
			}

		for (const auto& field : stdFieldData)
			if (e.Additional_.contains (field.ID_))
			{
				const auto& valMatcher = TypedMatcherBase::Create (field.Type_);
				valMatcher->SetValue (e.Additional_ [field.ID_].value<ANFieldValue> ());

				FieldMatch fieldMatch (field.Type_, valMatcher);
				fieldMatch.SetPluginID (sender);
				fieldMatch.SetFieldName (field.ID_);
				rule.AddFieldMatch (fieldMatch);
			}

		Rules_.prepend (rule);
		RulesModel_->insertRow (0, RuleToRow (rule));

		SaveSettings ();

		XmlSettingsManager::Instance ().ShowSettingsPage ("RulesWidget");
		emit focusOnRule (RulesModel_->index (0, 0));
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
			e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.AdvancedNotifications";
			e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = rule.GetCategory ();
			e.Additional_ ["org.LC.AdvNotifications.EventType"] = QStringList { rule.GetTypes ().toList () };
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
	}

	void RulesManager::LoadSettings ()
	{
		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AdvancedNotifications");
		settings.beginGroup ("rules");
		Rules_ = settings.value ("RulesList").value<QList<NotificationRule>> ();
		int rulesVersion = settings.value ("DefaultRulesVersion", 1).toInt ();

		const int currentDefVersion = 5;
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
		QStringList hrTypes;
		for (const QString& type : rule.GetTypes ())
			hrTypes << Type2HR_ [type];

		QList<QStandardItem*> items;
		items << new QStandardItem (rule.GetName ());
		items << new QStandardItem (Cat2HR_ [rule.GetCategory ()]);
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

	void RulesManager::prependRule ()
	{
		Rules_.prepend (NotificationRule ());
		RulesModel_->insertRow (0, RuleToRow (NotificationRule ()));
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
		Rules_ [idx].SetEnabled (enabled);
		if (auto item = RulesModel_->item (idx))
		{
			item->setData (enabled, RulesModel::Roles::IsRuleEnabled);
			item->setCheckState (enabled ? Qt::Checked : Qt::Unchecked);
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
