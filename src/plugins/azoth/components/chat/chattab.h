/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QPointer>
#include <QDateTime>
#include <util/sll/util.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/idndtab.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/iwkfontssettable.h>
#include "interfaces/azoth/azothcommon.h"
#include "ui_chattab.h"

class QTextBrowser;

namespace LC::Azoth::DndUtil
{
	struct DndContext;
}

namespace LC
{
namespace Util
{
	class FindNotificationWE;
	class WkFontsWidget;
}

namespace Azoth
{
	struct EntryStatus;

	class IAccount;
	class ICLEntry;
	class IMessage;
	class IMUCEntry;
	class ISupportPGP;
	class ITransferManager;

	class AvatarsManager;
	class CoreMessage;
	class MsgEditAutocompleter;
	class MsgFormatterWidget;

	class ChatTab : public QWidget
				  , public ITabWidget
				  , public IRecoverableTab
				  , public IDNDTab
				  , public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab IDNDTab IWkFontsSettable)

		static QObject *S_ParentMultiTabs_;
		static TabClassInfo S_ChatTabClass_;
		static TabClassInfo S_MUCTabClass_;

		AvatarsManager * const AvatarsManager_;
		const EntriesManager& Entries_;
		IAccount * const Account_;

		Ui::ChatTab Ui_;
		std::unique_ptr<QToolBar> TabToolbar_;
		QTextBrowser *MUCEventLog_;
		QAction *ToggleRichText_ = nullptr;
		QAction *Call_ = nullptr;
#ifdef ENABLE_CRYPT
		QAction *EnableEncryption_ = nullptr;
#endif

		std::optional<GlobalStrongestId> EntryId_;

		QList<QString> MsgHistory_;

		QList<IMessage*> HistoryMessages_;
		QDateTime LastDateTime_;
		QList<CoreMessage*> CoreMessages_;

		QIcon TabIcon_;

		MsgFormatterWidget *MsgFormatter_ = nullptr;
		MsgEditAutocompleter *MsgEditAutocompleter_ = nullptr;

		QString LastLink_;

		Util::FindNotificationWE *ChatFinder_;

		QImage LastAvatar_;

		Util::DefaultScopeGuard AvatarChangeSubscription_;

		int CurrentHistoryPosition_ = -1;

		int NumUnreadMsgs_ = 0;
		int ScrollbackPos_ = 0;
		bool HadHighlight_ = false;
		bool IsMUC_ = false;
		bool IsCurrent_ = false;
	public:
		static void SetParentMultiTabs (QObject*);
		static void SetChatTabClassInfo (const TabClassInfo&);
		static void SetMUCTabClassInfo (const TabClassInfo&);

		static const TabClassInfo& GetChatTabClassInfo ();
		static const TabClassInfo& GetMUCTabClassInfo ();

		ChatTab (const ICLEntry&,
				IAccount*,
				AvatarsManager*,
				const EntriesManager&,
				Util::WkFontsWidget*,
				QWebEngineProfile*,
				QWidget* = nullptr);
		~ChatTab ();

		void AddActionPane (QWidget&);

		/** Prepare (or update after it has been changed) the theme.
		 */
		void PrepareTheme ();

		/** Is called after the tab has been added to the tabwidget so
		 * that it could set its icon and various other stuff.
		 */
		void HasBeenAdded ();

		TabClassInfo GetTabClassInfo () const;
		QList<QAction*> GetTabBarContextMenuActions () const;
		QObject* ParentMultiTabs ();
		QToolBar* GetToolBar () const;
		void Remove ();
		void TabMadeCurrent ();
		void TabLostCurrent ();

		std::optional<TabSaveInfo> GetTabSaveInfo () const;

		void FillMimeData (QMimeData*);
		void HandleDragEnter (QDragMoveEvent*);
		void HandleDrop (QDropEvent*);

		QObject* GetQObject ();
		void SetFontFamily (FontFamily, const QFont&);
		void SetFontSize (FontSize type, int size);

		void ShowUsersList ();

		void HandleMUCParticipantsChanged ();

		void SetEnabled (bool);

		void UpdateEntryId (const std::optional<GlobalStrongestId>& id);

		std::optional<QString> GetSelectedVariant () const;

		QString ReformatTitle ();

		void ReinitEntry ();

		void ResyncLogs ();

		bool eventFilter (QObject*, QEvent*);

		template<typename T>
		T* GetEntry () const;
	public slots:
		void prepareMessageText (const QString&);
		void appendMessageText (const QString&);
		void insertMessageText (const QString&);
		void selectVariant (const QString&);
		QTextEdit* getMsgEdit ();

		void clearChat ();
	private slots:
		void on_MUCEventsButton__toggled (bool);
		void handleSeparateMUCLog (bool initial = false);

		void handleChatWindowSearch (const QString&);

		void messageSend ();
		void on_SubjectButton__toggled (bool);
		void on_SubjChange__released ();
		void on_View__loadFinished (bool);
		void handleHistoryBack ();
		void handleRichTextToggled ();
		void handleQuoteSelection ();
		void handleOpenLastLink ();
#ifdef ENABLE_MEDIACALLS
		void handleCallRequested ();
		void handleCall (QObject*);
#endif
#ifdef ENABLE_CRYPT
		void handleEnableEncryption ();
		void handleEncryptionStateChanged (QObject*, bool);
#endif
		void handleEntryMessage (QObject*);
		void handleVariantsChanged (QStringList);
		void handleStatusChanged (const EntryStatus&, const QString&);
		void handleChatPartStateChanged (const ChatPartState&, const QString&);
		void handleViewLinkClicked (QUrl, bool);
		void handleHistoryUp ();
		void handleHistoryDown ();

		void handleGotLastMessages (QObject*, const QList<QObject*>&);

		void handleSendButtonVisible ();

		void handleAccountStyleChanged (IAccount*);
	private:
		DndUtil::DndContext GetDndContext ();

		void BuildBasicActions ();
		void InitEntrySpecific ();
		void ReinitAvatar ();
		void ReinitEntryTypeSpecific ();

		void InitExtraActions ();
		void AddManagedActions (bool first);

		void InitMsgEdit (const ICLEntry&);
		void RegisterSettings ();

		void RequestLogs (int);

		/** Appends the message to the message view area.
		 */
		void AppendMessage (IMessage*);

		/** Updates the tab icon and other usages of state icon from the
		 * TabIcon_.
		 */
		void UpdateStateIcon ();

		/** Insert nickname into message edit field.
		 * @param nickname a nick to insert, in html format.
		 */
		void InsertNick (const QString& nicknameHtml);

#ifdef ENABLE_CRYPT
		void SetEncryptionEnabled (ISupportPGP*, bool enable);
#endif
	signals:
		void changeTabName (const QString&);
		void changeTabIcon (const QIcon&);
		void removeTab ();
		void raiseTab ();

		void needToClose ();

		void entryMadeCurrent (QObject*);
		void entryLostCurrent (QObject*);
		void messageSent ();

		void tabRecoverDataChanged ();

		void composingTextChanged (const QString&);
		void currentVariantChanged (const QString&);

		// Hooks
		void hookChatTabCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebEngineView *webView);
		void hookLinkClicked (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				const QUrl& linkUrl);
		void hookGonnaAppendMsg (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookThemeReloaded (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QWebEngineView *view,
				QObject *entry);
	};

	typedef QPointer<ChatTab> ChatTab_ptr;
}
}
