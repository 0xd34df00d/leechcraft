/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "core.h"
#include <QIcon>
#include <QAction>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDir>
#include <QMenu>
#include <QMetaMethod>
#include <QInputDialog>
#include <QMainWindow>
#include <QDomElement>
#include <QStringListModel>
#include <QMessageBox>
#include <QClipboard>
#include <QtDebug>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/xpc/defaulthookproxy.h>
#include <util/xpc/notificationactionhandler.h>
#include <util/xsd/wkfontswidget.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sys/resourceloader.h>
#include <util/sll/urloperator.h>
#include <util/sll/prelude.h>
#include <util/sll/util.h>
#include <util/sll/qtutil.h>
#include <util/threads/futures.h>
#include <interfaces/entityconstants.h>
#include <interfaces/iplugin2.h>
#include <interfaces/an/constants.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "interfaces/azoth/iprotocolplugin.h"
#include "interfaces/azoth/iprotocol.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iadvancedclentry.h"
#include "interfaces/azoth/imucentry.h"
#include "interfaces/azoth/imucperms.h"
#include "interfaces/azoth/iauthable.h"
#include "interfaces/azoth/iresourceplugin.h"
#include "interfaces/azoth/iurihandler.h"
#include "interfaces/azoth/irichtextmessage.h"
#include "interfaces/azoth/ihaveservicediscovery.h"
#include "interfaces/azoth/iextselfinfoaccount.h"
#include "interfaces/azoth/ihistoryplugin.h"
#include "interfaces/azoth/icanhavesslerrors.h"
#include "interfaces/azoth/ichatstyleresourcesource.h"

#ifdef ENABLE_CRYPT
#include "cryptomanager.h"
#endif

#include "components/dialogs/accounthandlerchooserdialog.h"
#include "components/dialogs/addcontactdialog.h"
#include "components/dialogs/joinconferencedialog.h"
#include "components/roster/clmodel.h"
#include "components/roster/cltooltipmanager.h"
#include "chattabsmanager.h"
#include "pluginmanager.h"
#include "proxyobject.h"
#include "xmlsettingsmanager.h"
#include "transferjobmanager.h"
#include "util.h"
#include "callmanager.h"
#include "actionsmanager.h"
#include "servicediscoverywidget.h"
#include "importmanager.h"
#include "unreadqueuemanager.h"
#include "chatstyleoptionmanager.h"
#include "riexhandler.h"
#include "customstatusesmanager.h"
#include "customchatstylemanager.h"
#include "corecommandsmanager.h"
#include "resourcesmanager.h"
#include "notificationsmanager.h"
#include "avatarsmanager.h"
#include "historysyncer.h"
#include "sslerrorshandler.h"
#include "roles.h"

Q_DECLARE_METATYPE (QPointer<QObject>);

namespace LC::Azoth
{
	QDataStream& operator<< (QDataStream& out, const EntryStatus& status)
	{
		quint8 version = 1;
		out << version
			<< static_cast<quint8> (status.State_)
			<< status.StatusString_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, EntryStatus& status)
	{
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		quint8 state;
		in >> state
			>> status.StatusString_;
		status.State_ = static_cast<State> (state);
		return in;
	}

	namespace
	{
		QByteArray GetStyleOptName (QObject *entry)
		{
			if (XmlSettingsManager::Instance ().property ("CustomMUCStyle").toBool () &&
					qobject_cast<IMUCEntry*> (entry))
				return "MUCWindowStyle";
			else
				return "ChatWindowStyle";
		}
	}

	QList<IAccount*> GetAccountsPred (const QObjectList& protocols,
			std::function<bool (IProtocol*)> protoPred = [] (IProtocol*) { return true; })
	{
		QList<IAccount*> accounts;
		for (const auto protoPlugin : protocols)
			for (const auto protoObj : qobject_cast<IProtocolPlugin*> (protoPlugin)->GetProtocols ())
			{
				const auto proto = qobject_cast<IProtocol*> (protoObj);
				if (!protoPred (proto))
					continue;

				for (const auto accountObj : proto->GetRegisteredAccounts ())
				{
					const auto acc = qobject_cast<IAccount*> (accountObj);
					if (!acc->IsShownInRoster ())
						continue;
					accounts << acc;
				}
			}
		return accounts;
	}

	Core::Core ()
	: Proxy_ (nullptr)
	, AvatarsManager_ (std::make_shared<AvatarsManager> ())
	, FontsWidget_ { new Util::WkFontsWidget { &XmlSettingsManager::Instance () } }
	, TooltipManager_ (new CLTooltipManager (AvatarsManager_.get (), Entry2Items_))
	, CLModel_ (new CLModel (TooltipManager_, this))
	, ChatTabsManager_ (new ChatTabsManager (AvatarsManager_.get (), FontsWidget_, this))
	, CoreCommandsManager_ (new CoreCommandsManager (this))
	, ActionsManager_ (new ActionsManager (AvatarsManager_.get (), this))
	, ItemIconManager_ (new AnimatedIconManager<QStandardItem*> ([] (QStandardItem *it, const QIcon& ic)
						{ it->setIcon (ic); }))
	, SmilesOptionsModel_ (new SourceTrackingModel<IEmoticonResourceSource> ({ tr ("Smile pack") }))
	, ChatStylesOptionsModel_ (new SourceTrackingModel<IChatStyleResourceSource> ({ tr ("Chat style") }))
	, PluginManager_ (new PluginManager)
	, PluginProxyObject_ (new ProxyObject (AvatarsManager_.get ()))
	, XferJobManager_ (new TransferJobManager { AvatarsManager_.get () })
	, CallManager_ (new CallManager)
	, ImportManager_ (new ImportManager)
	, UnreadQueueManager_ (new UnreadQueueManager)
	, CustomChatStyleManager_ (new CustomChatStyleManager)
	, HistorySyncer_ (std::make_shared<HistorySyncer> ())
	{
		FillANFields ();

		connect (this,
				SIGNAL (hookAddingCLEntryEnd (LC::IHookProxy_ptr, QObject*)),
				ChatTabsManager_,
				SLOT (handleAddingCLEntryEnd (LC::IHookProxy_ptr, QObject*)));
		connect (XferJobManager_.get (),
				SIGNAL (jobNoLongerOffered (QObject*)),
				this,
				SLOT (handleJobDeoffered (QObject*)));
		connect (ChatTabsManager_,
				SIGNAL (entryMadeCurrent (QObject*)),
				UnreadQueueManager_.get (),
				SLOT (clearMessagesForEntry (QObject*)));

		connect (UnreadQueueManager_.get (),
				SIGNAL (messagesCleared (QObject*)),
				this,
				SLOT (handleClearUnreadMsgCount (QObject*)));

		connect (AvatarsManager_.get (),
				&AvatarsManager::avatarInvalidated,
				this,
				[this] (QObject *entryObj)
				{
					Entry2SmoothAvatarCache_.remove (qobject_cast<ICLEntry*> (entryObj));
					UpdateItem (entryObj);
				});
		connect (AvatarsManager_.get (),
				&AvatarsManager::accountAvatarInvalidated,
				this,
				[this] (const IAccount *acc)
				{
					const auto item = GetAccountItem (acc);
					item->model ()->dataChanged (item->index (), item->index ());
				});

		PluginManager_->RegisterHookable (this);
		PluginManager_->RegisterHookable (CLModel_);
		PluginManager_->RegisterHookable (ActionsManager_);
		PluginManager_->RegisterHookable (TooltipManager_);

		SmilesOptionsModel_->AddModel (new QStringListModel (QStringList (QString ())));

		qRegisterMetaType<IMessage*> ("LC::Azoth::IMessage*");
		qRegisterMetaType<IMessage*> ("IMessage*");
		qRegisterMetaType<EntryStatus> ("LC::Azoth::EntryStatus");
		qRegisterMetaType<QPointer<QObject>> ("QPointer<QObject>");

		XmlSettingsManager::Instance ().RegisterObject ("StatusIcons",
				this, "updateStatusIconset");
		XmlSettingsManager::Instance ().RegisterObject ("GroupContacts",
				this, "handleGroupContactsChanged");
	}

	Core& Core::Instance ()
	{
		static Core c;
		return c;
	}

	void Core::Release ()
	{
		ShortcutManager_.reset ();
		StyleOptionManagers_.clear ();
		AvatarsManager_.reset ();

#ifdef ENABLE_CRYPT
		CryptoManager::Instance ().Release ();
#endif
	}

	void Core::SetProxy (ICoreProxy_ptr proxy, QObject *pluginObject)
	{
		Proxy_ = proxy;

		ShortcutManager_.reset (new Util::ShortcutManager (proxy, pluginObject));
		CustomStatusesManager_.reset (new CustomStatusesManager);

		NotificationsManager_.reset (new NotificationsManager (proxy->GetEntityManager (), AvatarsManager_.get ()));
		PluginManager_->RegisterHookable (NotificationsManager_.get ());
		connect (UnreadQueueManager_.get (),
				SIGNAL (messagesCleared (QObject*)),
				NotificationsManager_.get (),
				SLOT (handleClearUnreadMsgCount (QObject*)));

		connect (ChatTabsManager_,
				SIGNAL (entryMadeCurrent (QObject*)),
				NotificationsManager_.get (),
				SLOT (handleEntryMadeCurrent (QObject*)));

		auto addSOM = [this] (const QByteArray& option)
		{
			StyleOptionManagers_ [option].reset (new ChatStyleOptionManager (option, this));
		};
		addSOM ("ChatWindowStyle");
		addSOM ("MUCWindowStyle");
	}

	ICoreProxy_ptr Core::GetProxy () const
	{
		return Proxy_;
	}

	ProxyObject* Core::GetPluginProxy () const
	{
		return PluginProxyObject_.get ();
	}

	QList<ANFieldData> Core::GetANFields () const
	{
		return ANFields_;
	}

	QAbstractItemModel* Core::GetSmilesOptionsModel () const
	{
		return SmilesOptionsModel_.get ();
	}

	IEmoticonResourceSource* Core::GetCurrentEmoSource () const
	{
		const auto& pack = XmlSettingsManager::Instance ()
				.property ("SmileIcons").toString ();
		if (pack.isEmpty ())
			return nullptr;

		return SmilesOptionsModel_->GetSourceForOption (pack);
	}

	ChatStyleOptionManager* Core::GetChatStylesOptionsManager (const QByteArray& name) const
	{
		return StyleOptionManagers_ [name].get ();
	}

	Util::ShortcutManager* Core::GetShortcutManager () const
	{
		return ShortcutManager_.get ();
	}

	CustomStatusesManager* Core::GetCustomStatusesManager() const
	{
		return CustomStatusesManager_.get ();
	}

	CustomChatStyleManager* Core::GetCustomChatStyleManager () const
	{
		return CustomChatStyleManager_.get ();
	}

	UnreadQueueManager* Core::GetUnreadQueueManager () const
	{
		return UnreadQueueManager_.get ();
	}

	AvatarsManager* Core::GetAvatarsManager () const
	{
		return AvatarsManager_.get ();
	}

	Util::WkFontsWidget* Core::GetFontsWidget () const
	{
		return FontsWidget_;
	}

	void Core::AddPlugin (QObject *plugin)
	{
		IPlugin2 *plugin2 = qobject_cast<IPlugin2*> (plugin);
		if (!plugin2)
		{
			qWarning () << Q_FUNC_INFO
					<< plugin
					<< "isn't a IPlugin2";
			return;
		}

		QByteArray sig = QMetaObject::normalizedSignature ("initPlugin (QObject*)");
		if (plugin->metaObject ()->indexOfMethod (sig) != -1)
			QMetaObject::invokeMethod (plugin,
					"initPlugin",
					Q_ARG (QObject*, PluginProxyObject_.get ()));

		PluginManager_->AddPlugin (plugin);

		QSet<QByteArray> classes = plugin2->GetPluginClasses ();
		if (classes.contains ("org.LeechCraft.Plugins.Azoth.Plugins.IProtocolPlugin"))
			AddProtocolPlugin (plugin);

		if (classes.contains ("org.LeechCraft.Plugins.Azoth.Plugins.IResourceSourcePlugin"))
			AddResourceSourcePlugin (plugin);

		if (const auto ihp = qobject_cast<IHistoryPlugin*> (plugin))
			HistorySyncer_->AddStorage (ihp);
	}

	void Core::RegisterHookable (QObject *object)
	{
		PluginManager_->RegisterHookable (object);
	}

	bool Core::CouldHandle (const Entity& e) const
	{
		if (e.Mime_ == Mimes::PowerStateChanged ||
				e.Mime_ == "x-leechcraft/im-account-import" ||
				e.Mime_ == "x-leechcraft/im-history-import")
			return true;

		if (!e.Entity_.canConvert<QUrl> ())
			return false;

		const QUrl& url = e.Entity_.toUrl ();
		if (!url.isValid ())
			return false;

		return CouldHandleURL (url);
	}

	void Core::Handle (Entity e)
	{
		if (e.Mime_ == Mimes::PowerStateChanged)
		{
			HandlePowerNotification (e);
			return;
		}
		else if (e.Mime_ == "x-leechcraft/im-account-import")
		{
			ImportManager_->HandleAccountImport (e);
			return;
		}
		else if (e.Mime_ == "x-leechcraft/im-history-import")
		{
			ImportManager_->HandleHistoryImport (e);
			return;
		}

		const QUrl& url = e.Entity_.toUrl ();
		if (!url.isValid ())
			return;

		HandleURL (url);
	}

	bool Core::CouldHandleURL (const QUrl& url) const
	{
		for (const auto obj : ProtocolPlugins_)
		{
			const auto protoPlug = qobject_cast<IProtocolPlugin*> (obj);
			if (!protoPlug)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< obj
						<< "to IProtocolPlugin";
				continue;
			}

			for (const auto protoObj : protoPlug->GetProtocols ())
			{
				const auto handler = qobject_cast<IURIHandler*> (protoObj);
				if (handler && handler->SupportsURI (url))
					return true;
			}
		}

		return false;
	}

	void Core::HandleURL (const QUrl& url, ICLEntry *source)
	{
		QList<IAccount*> accounts;
		for (auto obj : ProtocolPlugins_)
		{
			auto protoPlug = qobject_cast<IProtocolPlugin*> (obj);
			if (!protoPlug)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< obj
						<< "to IProtocolPlugin";
				continue;
			}

			for (auto protoObj : protoPlug->GetProtocols ())
			{
				auto handler = qobject_cast<IURIHandler*> (protoObj);
				if (!handler)
					continue;
				if (!handler->SupportsURI (url))
					continue;

				auto proto = qobject_cast<IProtocol*> (protoObj);
				if (!proto)
				{
					qWarning () << Q_FUNC_INFO
							<< protoObj
							<< "doesn't implement IProtocol";
					continue;
				}
				for (const auto accObj : proto->GetRegisteredAccounts ())
					accounts << qobject_cast<IAccount*> (accObj);
			}
		}

		if (accounts.isEmpty ())
			return;

		if (source && accounts.contains (source->GetParentAccount ()))
		{
			accounts.clear ();
			accounts << source->GetParentAccount ();
		}

		IAccount *selected = nullptr;

		if (accounts.size () > 1)
		{
			AccountHandlerChooserDialog dia
			{
				accounts,
				tr ("Please select account to handle URI %1")
					.arg (url.toString ())
			};
			if (dia.exec () != QDialog::Accepted)
				return;

			selected = dia.GetSelectedAccount ();
		}
		else
			selected = accounts.at (0);

		if (!selected)
			return;

		const auto selProto = selected->GetParentProtocol ();
		qobject_cast<IURIHandler*> (selProto)->HandleURI (url, selected->GetQObject ());
	}

	void Core::HandleURLGeneric (QUrl url, bool raise, ICLEntry *source)
	{
		if (Core::Instance ().CouldHandleURL (url))
		{
			Core::Instance ().HandleURL (url, source);
			return;
		}

		if (url.scheme () == "file")
			return;

		if (url.scheme ().isEmpty () &&
				url.host ().isEmpty () &&
				url.path ().startsWith ("www."))
			url = "http://" + url.toString ();

		auto e = Util::MakeEntity (url,
				{},
				FromUserInitiated | OnlyHandle);
		if (!raise)
			e.Additional_ ["BackgroundHandle"] = true;
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	const QObjectList& Core::GetProtocolPlugins () const
	{
		return ProtocolPlugins_;
	}

	QAbstractItemModel* Core::GetCLModel () const
	{
		return CLModel_;
	}

	ChatTabsManager* Core::GetChatTabsManager () const
	{
		return ChatTabsManager_;
	}

	QList<IAccount*> Core::GetAccounts (std::function<bool (IProtocol*)> protoPred) const
	{
		return GetAccountsPred (ProtocolPlugins_, protoPred);
	}

	QList<IProtocol*> Core::GetProtocols () const
	{
		QList<IProtocol*> result;
		for (const auto protoPlugin : ProtocolPlugins_)
			for (const auto obj : qobject_cast<IProtocolPlugin*> (protoPlugin)->GetProtocols ())
				result << qobject_cast<IProtocol*> (obj);
		result.removeAll (nullptr);
		return result;
	}

	IAccount* Core::GetAccount (const QByteArray& id) const
	{
		for (const auto proto : GetProtocols ())
			for (const auto accObj : proto->GetRegisteredAccounts ())
			{
				const auto acc = qobject_cast<IAccount*> (accObj);
				if (acc && acc->GetAccountID () == id)
					return acc;
			}

		return nullptr;
	}

	void Core::UpdateItem (QObject *entryObj)
	{
		for (const auto item : Entry2Items_.value (qobject_cast<ICLEntry*> (entryObj)))
			CLModel_->dataChanged (item->index (), item->index ());
	}

	QStringList Core::GetChatGroups () const
	{
		QSet<QString> result;
		for (const auto pair : Util::Stlize (Entry2Items_))
		{
			const auto entry = pair.first;
			if (entry->GetEntryType () != ICLEntry::EntryType::Chat)
				continue;

			for (const auto& group : entry->Groups ())
				result << group;
		}
		return result.values ();
	}

	QObject* Core::GetEntry (const QString& id) const
	{
		return ID2Entry_.value (id);
	}

	void Core::OpenChat (const QModelIndex& contactIndex)
	{
		ChatTabsManager_->OpenChat (contactIndex);
	}

	TransferJobManager* Core::GetTransferJobManager () const
	{
		return XferJobManager_.get ();
	}

	CallManager* Core::GetCallManager () const
	{
		return CallManager_.get ();
	}

	SourceTrackingModel<IChatStyleResourceSource>* Core::GetChatStyleSourceModel () const
	{
		return ChatStylesOptionsModel_.get ();
	}

	bool Core::ShouldCountUnread (const ICLEntry *entry, IMessage *msg)
	{
		if (msg->GetQObject ()->property ("Azoth/HiddenMessage").toBool ())
			return false;

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookShouldCountUnread (proxy, msg->GetQObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toBool ();

		return !ChatTabsManager_->IsActiveChat (entry) &&
				msg->GetDirection () == IMessage::Direction::In &&
				(msg->GetMessageType () == IMessage::Type::ChatMessage ||
				 msg->GetMessageType () == IMessage::Type::MUCMessage);
	}

	bool Core::IsHighlightMessage (IMessage *msg)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookIsHighlightMessage (proxy, msg->GetQObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toBool ();

		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (msg->ParentCLEntry ());
		if (!mucEntry)
			return false;

		return msg->GetBody ().contains (mucEntry->GetNick (), Qt::CaseInsensitive);
	}

	void Core::AddProtocolPlugin (QObject *plugin)
	{
		IProtocolPlugin *ipp =
			qobject_cast<IProtocolPlugin*> (plugin);
		if (!ipp)
			qWarning () << Q_FUNC_INFO
				<< "plugin"
				<< plugin
				<< "tells it implements the IProtocolPlugin but cast failed";
		else
		{
			ProtocolPlugins_ << plugin;

			handleNewProtocols (ipp->GetProtocols ());

			connect (plugin,
					SIGNAL (gotNewProtocols (QList<QObject*>)),
					this,
					SLOT (handleNewProtocols (QList<QObject*>)));
		}
	}

	void Core::AddResourceSourcePlugin (QObject *rp)
	{
		const auto irp = qobject_cast<IResourcePlugin*> (rp);
		if (!irp)
		{
			qWarning () << Q_FUNC_INFO
					<< rp
					<< "doesn't implement IResourcePlugin";
			return;
		}

		for (const auto object : irp->GetResourceSources ())
		{
			if (const auto smileSrc = qobject_cast<IEmoticonResourceSource*> (object))
				AddSmileResourceSource (smileSrc);

			if (const auto chatStyleSrc = qobject_cast<IChatStyleResourceSource*> (object))
				AddChatStyleResourceSource (chatStyleSrc);
		}
	}

	void Core::AddSmileResourceSource (IEmoticonResourceSource *src)
	{
		SmilesOptionsModel_->AddSource (src);
	}

	void Core::AddChatStyleResourceSource (IChatStyleResourceSource *src)
	{
		ChatStylesOptionsModel_->AddSource (src);

		for (const auto& manager : StyleOptionManagers_)
			manager->AddChatStyleResourceSource (src);
	}

	QString Core::GetSelectedChatTemplate (QObject *entry, QWebEnginePage *page) const
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (entry);
		if (!src)
			return QString ();

		const auto& pair = CustomChatStyleManager_->GetForEntry (qobject_cast<ICLEntry*> (entry));
		if (!pair.first.isEmpty ())
			return src->GetHTMLTemplate (pair.first, pair.second, entry, page);

		const QByteArray& optName = GetStyleOptName (entry);
		const QString& opt = XmlSettingsManager::Instance ()
				.property (optName).toString ();
		const QString& var = XmlSettingsManager::Instance ()
				.property (optName + "Variant").toString ();
		return src->GetHTMLTemplate (opt, var, entry, page);
	}

	QUrl Core::GetSelectedChatTemplateURL (QObject *entry) const
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (entry);
		if (!src)
			return QUrl ();

		const auto& pair = CustomChatStyleManager_->GetForEntry (qobject_cast<ICLEntry*> (entry));
		if (!pair.first.isEmpty ())
			return pair.first;

		const QString& opt = XmlSettingsManager::Instance ()
				.property (GetStyleOptName (entry)).toString ();
		return src->GetBaseURL (opt);
	}

	bool Core::AppendMessageByTemplate (QWebEnginePage *page,
			QObject *message, const ChatMsgAppendInfo& info)
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (qobject_cast<IMessage*> (message)->ParentCLEntry ());
		if (!src)
		{
			qWarning () << Q_FUNC_INFO
					<< "empty result for"
					<< message;
			return false;
		}

		return src->AppendMessage (page, message, info);
	}

	void Core::FrameFocused (QObject *entry, QWebEnginePage *page)
	{
		IChatStyleResourceSource *src = GetCurrentChatStyle (entry);
		if (!src)
			return;

		src->FrameFocused (page);
	}

	QString Core::FormatDate (QDateTime dt, IMessage *msg)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFormatDateTime (proxy, this, dt, msg->GetQObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toString ();

		proxy->FillValue ("dateTime", dt);

		return dt.time ().toString ();
	}

	QString Core::FormatNickname (QString nick, IMessage *msg, const QString& color)
	{
		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookFormatNickname (proxy, this, nick, msg->GetQObject ());
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toString ();

		proxy->FillValue ("nick", nick);

		QString string;

		if (msg->GetMessageType () == IMessage::Type::MUCMessage)
		{
			QUrl url ("azoth://insertnick/");
			{
				Util::UrlOperator op { url };
				op ("nick", nick);
				if (const auto other = qobject_cast<ICLEntry*> (msg->OtherPart ()))
					op ("entryId", other->GetEntryID ());
			}

			string.append ("<span class='nickname'><a href=\"");
			string.append (url.toEncoded ());
			string.append ("\" class='nicklink' style='text-decoration:none; color:");
			string.append (color);
			string.append ("'>");
			string.append (nick);
			string.append ("</a></span>");
		}
		else
			string = QString ("<span class='nickname'>%1</span>")
					.arg (nick);

		return string;
	}

	namespace
	{
		bool IsInsideTag (const QString& body, int pos)
		{
			const auto prevOpen = body.lastIndexOf ('<', pos - 1);
			const auto nextClose = body.indexOf ('>', pos + 1);

			if (prevOpen == -1 || nextClose == -1)
				return false;

			const auto prevClose = body.lastIndexOf ('>', pos - 1);
			const auto nextOpen = body.indexOf ('<', pos + 1);

			return prevClose < prevOpen || nextClose < nextOpen;
		}

		void HighlightNicks (QString& body, IMessage *msg, const QList<QColor>& colors)
		{
			const auto entry = qobject_cast<IMUCEntry*> (msg->ParentCLEntry ());
			if (!entry)
				return;

			const auto intensity = XmlSettingsManager::Instance ()
					.property ("HighlightNicksInBodyAlphaReduction").toInt ();

			const auto& nicks = Util::Map (entry->GetParticipants (),
					[] (QObject *obj) { return qobject_cast<ICLEntry*> (obj)->GetEntryName (); });
			for (const auto& nick : nicks)
			{
				if (!body.contains (nick) || nick.isEmpty ())
					continue;

				auto nickColor = GetNickColor (nick, colors);
				if (nickColor.isNull ())
					continue;

				if (intensity != 100)
				{
					QColor color { nickColor };
					nickColor = QString ("rgba(%1, %2, %3, %4)")
							.arg (color.red ())
							.arg (color.green ())
							.arg (color.blue ())
							.arg (intensity / 100.);
				}

				auto isGoodChar = [] (const QChar& c) { return c.isSpace () || c.isPunct (); };

				int pos = 0;
				while ((pos = body.indexOf (nick, pos)) >= 0)
				{
					const auto posG = Util::MakeScopeGuard ([&pos, &nick] { pos += nick.size (); });

					if (IsInsideTag (body, pos))
						continue;

					const auto nickEnd = pos + nick.size ();
					if ((pos > 0 && !isGoodChar (body.at (pos - 1))) ||
						(nickEnd + 1 < body.size () && !isGoodChar (body.at (nickEnd))))
						continue;

					const auto& startStr = "<span style='color: " + nickColor + "'>";
					const QString endStr { "</span>" };
					body.insert (nickEnd, endStr);
					body.insert (pos, startStr);

					pos += startStr.size () + endStr.size ();
				}
			}
		}

		bool LimitImagesSize (const QDomNodeList& imgs)
		{
			const auto maxWidth = XmlSettingsManager::Instance ().property ("MaxImageWidth").toInt ();
			const auto maxHeight = XmlSettingsManager::Instance ().property ("MaxImageHeight").toInt ();

			for (int i = 0; i < imgs.size (); ++i)
			{
				auto img = imgs.at (i).toElement ();
				if (img.isNull ())
					continue;

				auto style = img.attribute ("style");
				style += QString { "; max-width: %1px; max-height: %2px;" }
						.arg (maxWidth)
						.arg (maxHeight);
				img.setAttribute ("style", style);
			}

			return true;
		}

		bool ReplaceImgsWithLinks (const QDomNodeList& imgs)
		{
			auto doc = imgs.at (0).ownerDocument ();

			struct Replacement
			{
				QDomElement NewElem_;
				QDomElement OldElem_;
			};
			QList<Replacement> replacements;
			for (int i = 0; i < imgs.size (); ++i)
			{
				auto img = imgs.at (i).toElement ();
				if (img.isNull ())
					continue;

				auto src = img.attribute ("src");
				if (src.isEmpty () || !src.startsWith ("http", Qt::CaseInsensitive))
					continue;

				auto link = doc.createElement ("a");
				link.setAttribute ("href", src);
				link.appendChild (doc.createTextNode (src));

				replacements.append ({ link, img });
			}

			for (const auto& rep : replacements)
				rep.OldElem_.parentNode ().replaceChild (rep.NewElem_, rep.OldElem_);

			return !replacements.isEmpty ();
		}

		void HandleImages (QString& body)
		{
			if (!body.contains ("img", Qt::CaseInsensitive))
				return;

			using Fun_t = bool (*) (const QDomNodeList&);
			QList<QPair<QByteArray, Fun_t>> processors
			{
				{ "ShowRichImagesAsLinks", &ReplaceImgsWithLinks },
				{ "LimitMaxImageSize", &LimitImagesSize }
			};

			auto ignorePair = [] (const auto& pair)
			{
				return !XmlSettingsManager::Instance ().property (pair.first).toBool ();
			};

			if (std::all_of (processors.begin (), processors.end (), ignorePair))
				return;

			QDomDocument doc;
			QString error;
			int errorLine;
			if (!doc.setContent (body, &error, &errorLine))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to parse the body as XML:"
						<< error
						<< errorLine
						<< "for body:\n"
						<< body;
				return;
			}

			const auto& imgs = doc.elementsByTagName ("img");
			if (imgs.isEmpty ())
				return;

			for (const auto& pair : processors)
			{
				if (ignorePair (pair))
					continue;

				if (!pair.second (imgs))
					continue;

				body = doc.toString (-1);
				break;
			}
		}

		void PostprocRichBody (QString& body)
		{
			HandleImages (body);
		}
	}

	QString Core::FormatBody (QString body, IMessage *msg, const QList<QColor>& colors)
	{
		QObject *msgObj = msg->GetQObject ();

		IRichTextMessage *rtMsg = qobject_cast<IRichTextMessage*> (msgObj);
		const bool isRich = rtMsg && rtMsg->GetRichBody () == body;

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		proxy->SetValue ("body", body);
		emit hookFormatBodyBegin (proxy, msgObj);
		if (proxy->IsCancelled ())
			return proxy->GetReturnValue ().toString ();

		proxy->FillValue ("body", body);

		if (!isRich)
		{
			PluginProxyObject_->GetFormatterProxy ().FormatLinks (body);
			body.replace ('\n', "<br />");
			body.replace ("  ", "&nbsp; ");
		}

		body = HandleSmiles (body);

		if (msg->GetMessageType () == IMessage::Type::MUCMessage &&
				XmlSettingsManager::Instance ().property ("HighlightNicksInBody").toBool ())
			HighlightNicks (body, msg, colors);

		if (isRich)
			PostprocRichBody (body);

		proxy.reset (new Util::DefaultHookProxy);
		proxy->SetValue ("body", body);
		emit hookFormatBodyEnd (proxy, msgObj);
		proxy->FillValue ("body", body);

		return proxy->IsCancelled () ?
				proxy->GetReturnValue ().toString () :
				body;
	}

	QString Core::HandleSmiles (QString body)
	{
		const QString& pack = XmlSettingsManager::Instance ()
				.property ("SmileIcons").toString ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookGonnaHandleSmiles (proxy, body, pack);
		if (proxy->IsCancelled ())
		{
			const QString& cand = proxy->GetReturnValue ().toString ();
			return cand.isEmpty () ? body : cand;
		}

		if (pack.isEmpty ())
			return body;

		IEmoticonResourceSource *src = SmilesOptionsModel_->GetSourceForOption (pack);
		if (!src)
			return body;

		const bool requireSpace = XmlSettingsManager::Instance ()
				.property ("RequireSpaceBeforeSmiles").toBool ();

		const QString& img = QString ("<img src=\"%2\" title=\"%1\" />");
		QMap<int, QString> pos2smile;
		for (const auto& str : src->GetEmoticonStrings (pack))
		{
			const auto& escaped = str.toHtmlEscaped ();
			int pos = 0;
			while ((pos = body.indexOf (escaped, pos)) != -1)
			{
				const bool isOk = !pos ||
						!requireSpace ||
						(requireSpace && pos && body [pos - 1].isSpace ());
				if (isOk)
					pos2smile [pos] = str;

				pos += escaped.size ();
			}
		}

		if (pos2smile.isEmpty ())
			return body;

		for (auto i = pos2smile.begin (); i != pos2smile.end (); ++i)
		{
			const auto& escapedSmile = i.value ().toHtmlEscaped ();
			for (int j = 1; j < escapedSmile.size (); ++j)
				pos2smile.remove (i.key () + j);
		}

		QList<QPair<int, QString>> reversed;
		reversed.reserve (pos2smile.size ());
		for (auto i = pos2smile.begin (); i != pos2smile.end (); ++i)
			reversed.push_front ({ i.key (), i.value () });

		for (const auto& pair : reversed)
		{
			const auto& str = pair.second;
			const auto& escaped = str.toHtmlEscaped ();

			const auto& rawData = src->GetImage (pack, str).toBase64 ();

			const auto& smileStr = img
					.arg (str)
					.arg (QString ("data:image/png;base64," + rawData));

			body.replace (pair.first, escaped.size (), smileStr);
		}

		return body;
	}

	namespace
	{
		QStringList GetDisplayGroups (const ICLEntry *clEntry)
		{
			QStringList groups;
			if (clEntry->GetEntryType () == ICLEntry::EntryType::UnauthEntry)
				groups << Core::tr ("Unauthorized users");
			else if (clEntry->GetEntryType () != ICLEntry::EntryType::Chat ||
					XmlSettingsManager::Instance ()
						.property ("GroupContacts").toBool ())
				groups = clEntry->Groups ();
			else
				groups << Core::tr ("Contacts");
			return groups;
		}
	}

	void Core::AddCLEntry (ICLEntry *clEntry,
			QStandardItem *accItem)
	{
		const auto entryObj = clEntry->GetQObject ();

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookAddingCLEntryBegin (proxy, entryObj);
		if (proxy->IsCancelled ())
			return;

		ResourcesManager::Instance ().HandleEntry (clEntry);

		connect (entryObj,
				SIGNAL (statusChanged (EntryStatus, QString)),
				this,
				SLOT (handleStatusChanged (EntryStatus, QString)));
		connect (entryObj,
				SIGNAL (availableVariantsChanged (QStringList)),
				this,
				SLOT (handleVariantsChanged ()));
		connect (entryObj,
				SIGNAL (gotMessage (QObject*)),
				this,
				SLOT (handleEntryGotMessage (QObject*)));
		connect (entryObj,
				SIGNAL (nameChanged (const QString&)),
				this,
				SLOT (handleEntryNameChanged (const QString&)));
		connect (entryObj,
				SIGNAL (groupsChanged (const QStringList&)),
				this,
				SLOT (handleEntryGroupsChanged (const QStringList&)));
		connect (entryObj,
				SIGNAL (permsChanged ()),
				this,
				SLOT (handleEntryPermsChanged ()));
		connect (entryObj,
				SIGNAL (entryGenerallyChanged ()),
				this,
				SLOT (updateItem ()));

		if (qobject_cast<IMUCEntry*> (entryObj))
		{
			connect (entryObj,
					SIGNAL (nicknameConflict (const QString&)),
					this,
					SLOT (handleNicknameConflict (const QString&)));
			connect (entryObj,
					SIGNAL (beenKicked (const QString&)),
					this,
					SLOT (handleBeenKicked (const QString&)));
			connect (entryObj,
					SIGNAL (beenBanned (const QString&)),
					this,
					SLOT (handleBeenBanned (const QString&)));
		}

		NotificationsManager_->AddCLEntry (entryObj);

#ifdef ENABLE_CRYPT
		CryptoManager::Instance ().AddEntry (clEntry);
#endif

		const QString& id = clEntry->GetEntryID ();
		ID2Entry_ [id] = entryObj;

		const auto& groups = GetDisplayGroups (clEntry);
		for (const auto catItem : GetCategoriesItems (groups, accItem))
		{
			AddEntryTo (clEntry, catItem);

			bool isMucCat = catItem->data (CLRIsMUCCategory).toBool ();
			if (!isMucCat)
				isMucCat = clEntry->GetEntryType () == ICLEntry::EntryType::PrivateChat;
			catItem->setData (isMucCat, CLRIsMUCCategory);
		}

		HandleStatusChanged (clEntry->GetStatus (), clEntry, QString ());

		if (clEntry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
			handleEntryPermsChanged (clEntry);

		TooltipManager_->AddEntry (clEntry);

		ChatTabsManager_->UpdateEntryMapping (id);

		proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookAddingCLEntryEnd (proxy, entryObj);
	}

	QList<QStandardItem*> Core::GetCategoriesItems (QStringList cats, QStandardItem *account)
	{
		if (cats.isEmpty ())
			cats << tr ("General");

		QList<QStandardItem*> result;
		for (const auto& cat : cats)
		{
			if (!Account2Category2Item_ [account].contains (cat))
			{
				QStandardItem *catItem = new QStandardItem (cat);
				catItem->setEditable (false);
				catItem->setData (account->data (CLRAccountObject), CLRAccountObject);
				catItem->setData (QVariant::fromValue<CLEntryType> (CLETCategory),
						CLREntryType);
				catItem->setData (cat, CLREntryCategory);
				catItem->setFlags (catItem->flags () | Qt::ItemIsDropEnabled);
				Account2Category2Item_ [account] [cat] = catItem;
				account->appendRow (catItem);
			}

			result << Account2Category2Item_ [account] [cat];
		}

		return result;
	}

	QStandardItem* Core::GetAccountItem (const IAccount *account)
	{
		for (int i = 0, size = CLModel_->rowCount ();
				i < size; ++i)
		{
			const auto& var = CLModel_->item (i)->data (CLRAccountObject);
			if (var.value<IAccount*> () == account)
				return CLModel_->item (i);
		}
		return 0;
	}

	QStandardItem* Core::GetAccountItem (const IAccount *account,
			QMap<const IAccount*, QStandardItem*>& accountItemCache)
	{
		if (accountItemCache.contains (account))
			return accountItemCache [account];

		const auto accountItem = GetAccountItem (account);
		if (accountItem)
			accountItemCache [account] = accountItem;
		return accountItem;
	}

	void Core::HandleStatusChanged (const EntryStatus&, ICLEntry *entry, const QString& variant)
	{
		emit hookEntryStatusChanged (Util::DefaultHookProxy_ptr (new Util::DefaultHookProxy),
				entry->GetQObject (), variant);

		const State state = entry->GetStatus ().State_;
		const auto& icon = ResourcesManager::Instance ().GetIconPathForState (state);

		for (auto item : Entry2Items_.value (entry))
		{
			ItemIconManager_->SetIcon (item, icon.get ());
			RecalculateOnlineForCat (item->parent ());
		}

		const QString& id = entry->GetEntryID ();
		if (!XferJobManager_->GetPendingIncomingJobsFor (id).isEmpty ())
			CheckFileIcon (id);
	}

	void Core::CheckFileIcon (const QString& id)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (GetEntry (id));
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "got null entry for"
					<< id;
			return;
		}

		if (XferJobManager_->GetPendingIncomingJobsFor (id).isEmpty ())
		{
			const QString& variant = entry->Variants ().value (0);
			HandleStatusChanged (entry->GetStatus (variant), entry, variant);
			return;
		}

		const QString& filename = XmlSettingsManager::Instance ()
				.property ("StatusIcons").toString () + "/file";
		const auto& fileIcon = ResourcesManager::Instance ()
				.GetResourceLoader (ResourcesManager::RLTStatusIconLoader)->
						GetIconDevice (filename, true);
		for (auto item : Entry2Items_.value (entry))
			ItemIconManager_->SetIcon (item, fileIcon.get ());
	}

	void Core::IncreaseUnreadCount (ICLEntry* entry, int amount)
	{
		for (auto item : Entry2Items_.value (entry))
			{
				int prevValue = item->data (CLRUnreadMsgCount).toInt ();
				item->setData (std::max (0, prevValue + amount), CLRUnreadMsgCount);
				RecalculateUnreadForParents (item);
			}
	}

	int Core::GetUnreadCount (ICLEntry *entry) const
	{
		const auto item = Entry2Items_.value (entry).value (0);
		return item ?
				item->data (CLRUnreadMsgCount).toInt () :
				0;
	}

	QImage Core::GetAvatar (ICLEntry *entry, int size)
	{
		if (!entry)
			return {};

		if (const auto candPtr = Entry2SmoothAvatarCache_ [entry])
		{
			const auto& cand = *candPtr;
			if (cand.width () == size ||
				cand.height () == size)
				return cand;

			if (cand.width () >= size ||
				cand.height () >= size)
				return cand.scaled ({ size, size }, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}

		const auto obj = entry->GetQObject ();
		Util::Sequence (obj, AvatarsManager_->GetAvatar (obj, IHaveAvatars::Size::Thumbnail)) >>
				[=, this] (QImage avatar)
				{
					avatar = avatar.isNull () ?
							QImage {} :
							avatar.scaled ({ size, size },
									Qt::KeepAspectRatio, Qt::SmoothTransformation);
					Entry2SmoothAvatarCache_.insert (entry, new QImage { avatar }, avatar.sizeInBytes ());

					UpdateItem (obj);
				};

		return ResourcesManager::Instance ().GetDefaultAvatar (size);
	}

	ActionsManager* Core::GetActionsManager () const
	{
		return ActionsManager_;
	}

	CoreCommandsManager* Core::GetCoreCommandsManager () const
	{
		return CoreCommandsManager_;
	}

	void Core::RecalculateUnreadForParents (QStandardItem *clItem)
	{
		QStandardItem *category = clItem->parent ();
		int sum = 0;
		for (int i = 0, rc = category->rowCount ();
				i < rc; ++i)
			sum += category->child (i)->data (CLRUnreadMsgCount).toInt ();
		category->setData (sum, CLRUnreadMsgCount);
	}

	void Core::RecalculateOnlineForCat (QStandardItem *catItem)
	{
		int result = 0;
		for (int i = 0; i < catItem->rowCount (); ++i)
		{
			auto entry = catItem->child (i)->data (CLRIEntry).value<ICLEntry*> ();
			result += entry->GetStatus ().State_ != SOffline;
		}

		catItem->setData (result, CLRNumOnline);
	}

	void Core::HandlePowerNotification (Entity e)
	{
		qDebug () << Q_FUNC_INFO << e.Entity_;

		if (e.Entity_ == PowerState::Sleeping)
			for (const auto acc : GetAccountsPred (ProtocolPlugins_))
			{
				const auto& state = acc->GetState ();
				if (state.State_ == SOffline)
					continue;

				SavedStatus_ [acc] = state;
				acc->ChangeState ({SOffline, tr ("Client went to sleep")});
			}
		else if (e.Entity_ == PowerState::WokeUp)
		{
			for (const auto& pair : Util::Stlize (SavedStatus_))
				pair.first->ChangeState (pair.second);
			SavedStatus_.clear ();
		}
	}

	void Core::RemoveCLItem (QStandardItem *item)
	{
		const auto entry = item->data (CLRIEntry).value<ICLEntry*> ();
		Entry2Items_ [entry].removeAll (item);

		QStandardItem *category = item->parent ();
		const int unread = item->data (CLRUnreadMsgCount).toInt ();

		ItemIconManager_->Cancel (item);

		category->removeRow (item->row ());

		if (!category->rowCount ())
		{
			QStandardItem *account = category->parent ();
			ItemIconManager_->Cancel (category);

			const QString& text = category->text ();

			account->removeRow (category->row ());
			Account2Category2Item_ [account].remove (text);
		}
		else if (unread)
		{
			const int sum = category->data (CLRUnreadMsgCount).toInt ();
			category->setData (std::max (sum - unread, 0), CLRUnreadMsgCount);
		}
	}

	void Core::AddEntryTo (ICLEntry *clEntry, QStandardItem *catItem)
	{
		QStandardItem *clItem = new QStandardItem (clEntry->GetEntryName ());
		clItem->setEditable (false);
		const auto acc = clEntry->GetParentAccount ();
		clItem->setData (QVariant::fromValue<IAccount*> (acc), CLRAccountObject);
		clItem->setData (QVariant::fromValue<QObject*> (clEntry->GetQObject ()), CLREntryObject);
		clItem->setData (QVariant::fromValue<ICLEntry*> (clEntry), CLRIEntry);
		clItem->setData (QVariant::fromValue<CLEntryType> (CLETContact), CLREntryType);
		clItem->setData (catItem->data (CLREntryCategory), CLREntryCategory);

		clItem->setFlags (clItem->flags () |
				Qt::ItemIsDragEnabled |
				Qt::ItemIsDropEnabled);

		catItem->appendRow (clItem);

		Entry2Items_ [clEntry] << clItem;
	}

	IChatStyleResourceSource* Core::GetCurrentChatStyle (QObject *entry) const
	{
		const auto& pair = CustomChatStyleManager_->GetForEntry (qobject_cast<ICLEntry*> (entry));
		if (!pair.first.isEmpty ())
			if (auto src = ChatStylesOptionsModel_->GetSourceForOption (pair.first))
				return src;

		const QString& opt = XmlSettingsManager::Instance ()
				.property (GetStyleOptName (entry)).toString ();
		auto src = ChatStylesOptionsModel_->GetSourceForOption (opt);
		if (!src)
			qWarning () << Q_FUNC_INFO
					<< "empty result for"
					<< opt;
		return src;
	}

	void Core::FillANFields ()
	{
		const QStringList havingMsgField
		{
			AN::TypeIMMUCHighlight,
			AN::TypeIMMUCMsg,
			AN::TypeIMIncMsg,
			AN::TypeIMIncFile,
			AN::TypeIMAttention,
			AN::TypeIMSubscrGrant,
			AN::TypeIMSubscrRevoke,
			AN::TypeIMSubscrRequest
		};

		const QStringList havingSourceFields
		{
			AN::TypeIMMUCHighlight,
			AN::TypeIMMUCMsg,
			AN::TypeIMIncMsg,
			AN::TypeIMIncFile,
			AN::TypeIMAttention,
			AN::TypeIMSubscrGrant,
			AN::TypeIMSubscrRevoke,
			AN::TypeIMSubscrRequest,
			AN::TypeIMStatusChange,
			AN::TypeIMEventTuneChange,
			AN::TypeIMEventMoodChange,
			AN::TypeIMEventActivityChange,
			AN::TypeIMEventLocationChange
		};

		ANFields_ = QList<ANFieldData>
		{
			{
				"org.LC.Plugins.Azoth.Msg",
				tr ("Message body"),
				tr ("Original human-readable message body."),
				QVariant::String,
				[&]
				{
					auto res = havingMsgField + havingSourceFields;
					res.removeDuplicates ();
					return res;
				} ()
			},
			{
				"org.LC.Plugins.Azoth.SourceName",
				tr ("Sender name"),
				tr ("Human-readable name of the sender of the message."),
				QVariant::String,
				havingSourceFields
			},
			{
				"org.LC.Plugins.Azoth.SourceID",
				tr ("Sender ID"),
				tr ("Non-human-readable ID of the sender (protocol-specific)."),
				QVariant::String,
				havingSourceFields
			},
			{
				"org.LC.Plugins.Azoth.ParentSourceName",
				tr ("Sender's parent entry name"),
				tr ("Human-readable name of the parent entry of the sender of the message, like MUC name for a chat participant."),
				QVariant::String,
				havingSourceFields
			},
			{
				"org.LC.Plugins.Azoth.ParentSourceID",
				tr ("Sender's parent ID"),
				tr ("Non-human-readable ID of the parent entry of the sender of the message, like MUC name for a chat participant."),
				QVariant::String,
				havingSourceFields
			},
			{
				"org.LC.Plugins.Azoth.SourceGroups",
				tr ("Sender groups"),
				tr ("Groups to which the sender belongs."),
				QVariant::StringList,
				havingSourceFields
			},
			{
				"org.LC.Plugins.Azoth.NewStatus",
				tr ("New status"),
				tr ("The new status string of the contact."),
				QVariant::String,
				{ AN::TypeIMStatusChange }
			}
		};
	}

	void Core::handleMucJoinRequested ()
	{
		auto accounts = GetAccountsPred (ProtocolPlugins_,
				[] (IProtocol *proto) { return proto->GetFeatures () & IProtocol::PFMUCsJoinable; });

		auto rootWM = GetProxy ()->GetRootWindowsManager ();
		auto dia = new JoinConferenceDialog (accounts, rootWM->GetPreferredWindow ());
		dia->show ();
	}

	void Core::handleShowNextUnread ()
	{
		UnreadQueueManager_->ShowNext ();
	}

	void Core::saveAccountVisibility (IAccount *account)
	{
		const auto& id = "ShowAccount_" + account->GetAccountID ();
		XmlSettingsManager::Instance ().setProperty (id, account->IsShownInRoster ());
	}

	void Core::handleNewProtocols (const QList<QObject*>& protocols)
	{
		for (const auto protoObj : protocols)
		{
			const auto proto = qobject_cast<IProtocol*> (protoObj);

			for (const auto accObj : proto->GetRegisteredAccounts ())
				addAccount (accObj);

			connect (proto->GetQObject (),
					SIGNAL (accountAdded (QObject*)),
					this,
					SLOT (addAccount (QObject*)));
			connect (proto->GetQObject (),
					SIGNAL (accountRemoved (QObject*)),
					this,
					SLOT (handleAccountRemoved (QObject*)));
		}
	}

	namespace
	{
		std::optional<EntryStatus> LoadSavedStatus (IAccount *account)
		{
			const auto proto = qobject_cast<IProtocol*> (account->GetParentProtocol ());
			if (!proto)
			{
				qWarning () << Q_FUNC_INFO
						<< "account's parent proto isn't IProtocol"
						<< account->GetParentProtocol ();
				return {};
			}

			const auto& id = proto->GetProtocolID () + account->GetAccountID ();
			const auto& var = XmlSettingsManager::Instance ().property (id);
			if (var.isNull () || !var.canConvert<QByteArray> ())
				return {};

			EntryStatus s;
			QDataStream stream { var.toByteArray () };
			stream >> s;

			if (s.State_ == State::SConnecting)
				s.State_ = State::SOnline;

			return s;
		}
	}

	void Core::addAccount (QObject *accObject)
	{
		AvatarsManager_->handleAccount (accObject);

		IAccount *account = qobject_cast<IAccount*> (accObject);
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< accObject
					<< sender ();
			return;
		}

		HistorySyncer_->AddAccount (account);

		const auto& accountId = account->GetAccountID ();
		const auto& accountName = account->GetAccountName ();

		const auto& showKey = "ShowAccount_" + accountId.toStdString ();
		const bool show = XmlSettingsManager::Instance ().Property (showKey, true).toBool ();
		account->SetShownInRoster (show);

		emit accountAdded (account);

#ifdef ENABLE_CRYPT
		CryptoManager::Instance ().AddAccount (account);
#endif

		QStandardItem *accItem = new QStandardItem (accountName);
		accItem->setData (QVariant::fromValue<IAccount*> (account), CLRAccountObject);
		accItem->setData (QVariant::fromValue<CLEntryType> (CLETAccount), CLREntryType);
		const auto accState = account->GetState ().State_;
		ItemIconManager_->SetIcon (accItem,
				ResourcesManager::Instance ().GetIconPathForState (accState).get ());

		CLModel_->appendRow (accItem);

		accItem->setEditable (false);

		QList<QStandardItem*> clItems;
		for (const auto clObj : account->GetCLEntries ())
		{
			const auto clEntry = qobject_cast<ICLEntry*> (clObj);
			if (!clEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< "entry doesn't implement ICLEntry"
						<< clObj
						<< account;
				continue;
			}

			AddCLEntry (clEntry, accItem);
		}

		NotificationsManager_->AddAccount (accObject);

		connect (accObject,
				SIGNAL (gotCLItems (const QList<QObject*>&)),
				this,
				SLOT (handleGotCLItems (const QList<QObject*>&)));
		connect (accObject,
				SIGNAL (removedCLItems (const QList<QObject*>&)),
				this,
				SLOT (handleRemovedCLItems (const QList<QObject*>&)));

		connect (accObject,
				SIGNAL (statusChanged (const EntryStatus&)),
				this,
				SLOT (handleAccountStatusChanged (const EntryStatus&)));

		connect (accObject,
				SIGNAL (accountRenamed (const QString&)),
				this,
				SLOT (handleAccountRenamed (const QString&)));

		if (qobject_cast<IHaveServiceDiscovery*> (accObject))
			connect (accObject,
					SIGNAL (gotSDSession (QObject*)),
					this,
					SLOT (handleGotSDSession (QObject*)));

		if (account->IsShownInRoster ())
		{
			if (const auto& s = LoadSavedStatus (account))
				account->ChangeState (*s);
		}

		if (const auto xferMgr = account->GetTransferManager ())
		{
			XferJobManager_->AddAccountManager (xferMgr);

			connect (xferMgr,
					SIGNAL (fileOffered (QObject*)),
					this,
					SLOT (handleFileOffered (QObject*)));
		}

		CallManager_->AddAccount (account->GetQObject ());

		if (qobject_cast<ISupportRIEX*> (account->GetQObject ()))
			connect (account->GetQObject (),
					SIGNAL (riexItemsSuggested (QList<LC::Azoth::RIEXItem>, QObject*, QString)),
					this,
					SLOT (handleRIEXItemsSuggested (QList<LC::Azoth::RIEXItem>, QObject*, QString)));

		if (const auto ichse = qobject_cast<ICanHaveSslErrors*> (account->GetQObject ()))
			new SslErrorsHandler { SslErrorsHandler::Account { accountId, accountName }, ichse };
	}

	void Core::handleAccountRemoved (QObject *account)
	{
		auto accFace = qobject_cast<IAccount*> (account);
		if (!accFace)
		{
			qWarning () << Q_FUNC_INFO
					<< "account doesn't implement IAccount*"
					<< account
					<< sender ();
			return;
		}

		emit accountRemoved (accFace);

		for (int i = 0; i < CLModel_->rowCount (); ++i)
		{
			QStandardItem *item = CLModel_->item (i);
			const auto obj = item->data (CLRAccountObject).value<IAccount*> ();
			if (obj == accFace)
			{
				ItemIconManager_->Cancel (item);
				CLModel_->removeRow (i);
				break;
			}
		}

		for (auto entry : Entry2Items_.keys ())
			if (entry->GetParentAccount () == accFace)
				Entry2Items_.remove (entry);

		NotificationsManager_->RemoveAccount (account);

		disconnect (account,
				0,
				this,
				0);
	}

	void Core::handleGotCLItems (const QList<QObject*>& items)
	{
		QMap<const IAccount*, QStandardItem*> accountItemCache;
		for (const auto item : items)
		{
			const auto entry = qobject_cast<ICLEntry*> (item);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< item
						<< "from"
						<< sender ()
						<< "is not a valid ICLEntry";
				continue;
			}

			if (Entry2Items_.contains (entry))
				continue;

			const auto account = entry->GetParentAccount ();
			const auto accountItem = GetAccountItem (account, accountItemCache);
			if (!accountItem)
			{
				qWarning () << Q_FUNC_INFO
						<< "could not find account item for"
						<< item
						<< account->GetAccountID ();
				continue;
			}

			AddCLEntry (entry, accountItem);

			if (entry->GetEntryType () == ICLEntry::EntryType::MUC)
			{
				auto mucEntry = qobject_cast<IMUCEntry*> (item);

				const bool open = XmlSettingsManager::Instance ()
						.property ("OpenTabsForAutojoin").toBool ();
				if (open || !mucEntry->IsAutojoined ())
				{
					auto item = Entry2Items_.value (entry).first ();
					OpenChat (CLModel_->indexFromItem (item));
				}
			}

			ChatTabsManager_->HandleEntryAdded (entry);
		}
	}

	void Core::handleRemovedCLItems (const QList<QObject*>& items)
	{
		for (const auto clitem : items)
		{
			const auto entry = qobject_cast<ICLEntry*> (clitem);
			if (!entry)
			{
				qWarning () << Q_FUNC_INFO
						<< clitem
						<< "is not a valid ICLEntry";
				continue;
			}

			if (entry->GetEntryType () == ICLEntry::EntryType::MUC &&
					XmlSettingsManager::Instance ().property ("CloseConfOnLeave").toBool ())
				GetChatTabsManager ()->CloseChat (entry, false);

			disconnect (clitem,
					0,
					this,
					0);

			TooltipManager_->RemoveEntry (entry);

			ChatTabsManager_->HandleEntryRemoved (entry);

			for (auto item : Entry2Items_.value (entry))
				RemoveCLItem (item);

			Entry2Items_.remove (entry);

			ActionsManager_->HandleEntryRemoved (entry);

			ID2Entry_.remove (entry->GetEntryID ());

			Entry2SmoothAvatarCache_.remove (entry);

			NotificationsManager_->RemoveCLEntry (clitem);

			ResourcesManager::Instance ().HandleRemoved (entry);
		}
	}

	void Core::handleAccountStatusChanged (const EntryStatus& status)
	{
		IAccount *acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not an IAccount"
					<< sender ();
			return;
		}

		IProtocol *proto = qobject_cast<IProtocol*> (acc->GetParentProtocol ());
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "account's proto is not a IProtocol"
					<< acc->GetParentProtocol ();
			return;
		}

		const QByteArray& id = proto->GetProtocolID () + acc->GetAccountID ();
		QByteArray serializedStatus;
		{
			QDataStream stream (&serializedStatus, QIODevice::WriteOnly);
			stream << status;
		}
		XmlSettingsManager::Instance ().setProperty (id,
				serializedStatus);

		for (int i = 0, size = CLModel_->rowCount (); i < size; ++i)
		{
			QStandardItem *item = CLModel_->item (i);
			if (item->data (CLRAccountObject).value<IAccount*> () != acc)
				continue;

			ItemIconManager_->SetIcon (item,
					ResourcesManager::Instance ().GetIconPathForState (status.State_).get ());
			return;
		}

		qWarning () << Q_FUNC_INFO
				<< "item for account"
				<< sender ()
				<< "not found";
	}

	void Core::handleAccountRenamed (const QString& name)
	{
		const auto acc = qobject_cast<IAccount*> (sender ());
		if (!acc)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not an IAccount"
					<< sender ();
			return;
		}

		for (int i = 0, size = CLModel_->rowCount (); i < size; ++i)
		{
			QStandardItem *item = CLModel_->item (i);
			if (item->data (CLRAccountObject).value<IAccount*> () != acc)
				continue;

			item->setText (name);
			return;
		}
	}

	void Core::handleStatusChanged (const EntryStatus& status, const QString& variant)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a ICLEntry:"
					<< sender ();
			return;
		}

		HandleStatusChanged (status, entry, variant);
	}

	void Core::handleVariantsChanged ()
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a ICLEntry:"
					<< sender ();
			return;
		}

		HandleStatusChanged (entry->GetStatus (), entry, {});
	}

	void Core::handleEntryNameChanged (const QString& newName)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender is not a ICLEntry:"
					<< sender ();
			return;
		}

		for (auto item : Entry2Items_.value (entry))
			item->setText (newName);

		if (entry->Variants ().size ())
			HandleStatusChanged (entry->GetStatus (), entry, entry->Variants ().first ());
	}

	void Core::handleEntryGroupsChanged (QStringList newGroups, QObject *perform)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (perform ? perform : sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "could not be casted to ICLEntry";
			return;
		}

		if (entry->GetEntryType () == ICLEntry::EntryType::Chat)
			newGroups = GetDisplayGroups (entry);

		if (!Entry2Items_.contains (entry))
			return;

		for (auto item : Entry2Items_.value (entry))
		{
			const QString& oldCat = item->data (CLREntryCategory).toString ();
			if (newGroups.removeAll (oldCat))
				continue;

			RemoveCLItem (item);
		}

		if (newGroups.isEmpty () && !Entry2Items_.value (entry).isEmpty ())
			return;

		auto accItem = GetAccountItem (entry->GetParentAccount ());

		for (auto catItem : GetCategoriesItems (newGroups, accItem))
			AddEntryTo (entry, catItem);

		HandleStatusChanged (entry->GetStatus (), entry, QString ());
	}

	void Core::handleEntryPermsChanged (ICLEntry *suggest)
	{
		ICLEntry *entry = suggest ? suggest : qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "could not be casted to ICLEntry";
			return;
		}

		QObject *entryObj = entry->GetQObject ();
		const auto mucPerms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntryObject ());
		if (!mucPerms)
			return;

		const QString& name = mucPerms->GetAffName (entryObj);
		for (auto item : Entry2Items_.value (entry))
			item->setData (name, CLRAffiliation);
	}

	void Core::handleEntryGotMessage (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());

		if (!other && msg->OtherPart ())
		{
			qWarning () << Q_FUNC_INFO
					<< "message's other part cannot be cast to ICLEntry"
					<< msg->OtherPart ();
			return;
		}

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		emit hookGotMessage (proxy, msgObj);
		if (proxy->IsCancelled ())
			return;

		proxy.reset (new Util::DefaultHookProxy);
		emit hookGotMessage2 (proxy, msgObj);

		if (msg->GetMessageType () != IMessage::Type::MUCMessage &&
				msg->GetMessageType () != IMessage::Type::ChatMessage)
			return;

		ICLEntry *parentCL = qobject_cast<ICLEntry*> (msg->ParentCLEntry ());

		if (ShouldCountUnread (parentCL, msg))
		{
			IncreaseUnreadCount (parentCL);
			UnreadQueueManager_->AddMessage (msgObj);
		}

		if (msg->GetDirection () != IMessage::Direction::In ||
				ChatTabsManager_->IsActiveChat (parentCL))
			return;

		ChatTabsManager_->HandleInMessage (msg);
		NotificationsManager_->HandleMessage (msg);
	}

	void Core::handleNicknameConflict (const QString& usedNick)
	{
		ICLEntry *clEntry = qobject_cast<ICLEntry*> (sender ());
		IMUCEntry *entry = qobject_cast<IMUCEntry*> (sender ());
		if (!entry || !clEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLEntry or IMUCEntry";
			return;
		}

		QString altNick;
		if (XmlSettingsManager::Instance ().property ("UseAltNick").toBool ())
		{
			QString append = XmlSettingsManager::Instance ()
				.property ("AlternativeNickname").toString ();
			if (append.isEmpty ())
				append = "_azoth";
			altNick = usedNick + append;
		}

		if ((altNick.isEmpty () || altNick == usedNick) &&
				QMessageBox::question (0,
						tr ("Nickname conflict"),
						tr ("You have specified a nickname for %1 that's "
							"already used. Would you like to try to "
							"join with another nick?")
							.arg (clEntry->GetEntryName ()),
						QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		const QString& newNick = altNick.isEmpty () || altNick == usedNick ?
				QInputDialog::getText (0,
						tr ("Enter new nick"),
						tr ("Enter new nick for joining %1 (%2 is already used):")
							.arg (clEntry->GetEntryName ())
							.arg (usedNick),
						QLineEdit::Normal,
						usedNick) :
				altNick;
		if (newNick.isEmpty ())
			return;

		entry->SetNick (newNick);
		entry->Join ();
	}

	void Core::handleBeenKicked (const QString& reason)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		IMUCEntry *mucEntry = qobject_cast<IMUCEntry*> (sender ());
		if (!entry || !mucEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLEntry or IMUCEntry";
			return;
		}

		const QString& text = reason.isEmpty () ?
				tr ("You have been kicked from %1. Do you want to rejoin?")
					.arg (entry->GetEntryName ()) :
				tr ("You have been kicked from %1: %2. Do you want to rejoin?")
					.arg (entry->GetEntryName ())
					.arg (reason);

		if (QMessageBox::question (0,
				"LeechCraft Azoth",
				text,
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			mucEntry->Join ();
	}

	void Core::handleBeenBanned (const QString& reason)
	{
		ICLEntry* entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "doesn't implement ICLEntry";
			return;
		}

		const QString& text = reason.isEmpty () ?
				tr ("You have been banned from %1.")
					.arg (entry->GetEntryName ()) :
				tr ("You have been banned from %1: %2.")
					.arg (entry->GetEntryName ())
					.arg (reason);
		QMessageBox::warning (0,
				"LeechCraft Azoth",
				text);
	}

	void Core::updateStatusIconset ()
	{
		QMap<State, Util::QIODevice_ptr> state2IconCache;
		for (const auto entry : Entry2Items_.keys ())
		{
			const auto state = entry->GetStatus ().State_;
			if (!state2IconCache.contains (state))
				state2IconCache [state] = ResourcesManager::Instance ().GetIconPathForState (state);

			for (auto item : Entry2Items_.value (entry))
			{
				const auto& dev = state2IconCache [state];
				ItemIconManager_->SetIcon (item, dev.get ());
			}
		}
	}

	void Core::handleGroupContactsChanged ()
	{
		for (const auto& pair : Util::Stlize (Entry2Items_))
		{
			const auto entry = pair.first;
			if (entry->GetEntryType () == ICLEntry::EntryType::Chat)
				handleEntryGroupsChanged (GetDisplayGroups (entry), entry->GetQObject ());
		}
	}

	void Core::updateItem ()
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "sender doesn't implement ICLEntry"
					<< sender ();
			return;
		}

		for (auto item : Entry2Items_.value (entry))
			item->model ()->dataChanged (item->index (), item->index ());
	}

	void Core::handleClearUnreadMsgCount (QObject *entryObj)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		for (auto item : Entry2Items_.value (entry))
		{
			item->setData (0, CLRUnreadMsgCount);
			RecalculateUnreadForParents (item);
		}
	}

	void Core::handleGotSDSession (QObject *sdObj)
	{
		ISDSession *sess = qobject_cast<ISDSession*> (sdObj);
		if (!sess)
		{
			qWarning () << Q_FUNC_INFO
					<< sdObj
					<< "is not a ISDSession";
			return;
		}

		const auto w = new ServiceDiscoveryWidget;
		w->SetAccount (sender ());
		w->SetSDSession (sess);
		GetProxyHolder ()->GetRootWindowsManager ()->AddTab (w);
	}

	void Core::handleFileOffered (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "could not be casted to ITransferJob";
			return;
		}

		const QString& id = job->GetSourceID ();
		IncreaseUnreadCount (qobject_cast<ICLEntry*> (GetEntry (id)));

		CheckFileIcon (id);
	}

	void Core::handleJobDeoffered (QObject *jobObj)
	{
		ITransferJob *job = qobject_cast<ITransferJob*> (jobObj);
		if (!job)
		{
			qWarning () << Q_FUNC_INFO
					<< jobObj
					<< "could not be casted to ITransferJob";
			return;
		}

		const QString& id = job->GetSourceID ();
		IncreaseUnreadCount (qobject_cast<ICLEntry*> (GetEntry (id)), -1);
		CheckFileIcon (id);
	}

	void Core::handleRIEXItemsSuggested (QList<RIEXItem> items, QObject *from, QString message)
	{
		RIEX::HandleRIEXItemsSuggested (items, from, message);
	}
}
