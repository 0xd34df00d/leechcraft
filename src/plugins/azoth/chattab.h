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
#include <QPersistentModelIndex>
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

namespace LC
{
namespace Util
{
	class FindNotificationWk;
	class WkFontsWidget;
}

namespace Azoth
{
	struct EntryStatus;
	class CoreMessage;
	class ICLEntry;
	class IMUCEntry;
	class IAccount;
	class IMessage;
	class ITransferManager;
	class ISupportPGP;
	class ContactDropFilter;
	class MsgFormatterWidget;
	class AvatarsManager;

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
		IAccount * const Account_;

		Ui::ChatTab Ui_;
		std::unique_ptr<QToolBar> TabToolbar_;
		QTextBrowser *MUCEventLog_;
		QAction *ToggleRichEditor_ = nullptr;
		QAction *ToggleRichText_ = nullptr;
		QAction *Call_ = nullptr;
#ifdef ENABLE_CRYPT
		QAction *EnableEncryption_ = nullptr;
#endif

		const QString EntryID_;

		QColor BgColor_;

		QList<QString> MsgHistory_;
		int CurrentHistoryPosition_ = -1;

		bool HadHighlight_ = false;
		int NumUnreadMsgs_ = 0;
		int ScrollbackPos_ = 0;

		QList<IMessage*> HistoryMessages_;
		QDateTime LastDateTime_;
		QList<CoreMessage*> CoreMessages_;

		QIcon TabIcon_;
		bool IsMUC_ = false;
		int PreviousTextHeight_ = 0;

		ContactDropFilter *CDF_;
		MsgFormatterWidget *MsgFormatter_ = nullptr;

		QString LastLink_;

		Util::FindNotificationWk *ChatFinder_;

		bool IsCurrent_ = false;

		QImage LastAvatar_;

		Util::DefaultScopeGuard AvatarChangeSubscription_;
	public:
		static void SetParentMultiTabs (QObject*);
		static void SetChatTabClassInfo (const TabClassInfo&);
		static void SetMUCTabClassInfo (const TabClassInfo&);

		static const TabClassInfo& GetChatTabClassInfo ();
		static const TabClassInfo& GetMUCTabClassInfo ();

		ChatTab (const QString&,
				IAccount*,
				AvatarsManager*,
				Util::WkFontsWidget*,
				QWebEngineProfile*,
				QWidget* = nullptr);
		~ChatTab ();

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

		QByteArray GetTabRecoverData () const;
		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;

		void FillMimeData (QMimeData*);
		void HandleDragEnter (QDragMoveEvent*);
		void HandleDrop (QDropEvent*);

		QObject* GetQObject ();
		void SetFontFamily (FontFamily, const QFont&);
		void SetFontSize (FontSize type, int size);

		void ShowUsersList ();

		void HandleMUCParticipantsChanged ();

		void SetEnabled (bool);

		QObject* GetCLEntry () const;
		QString GetEntryID () const;
		QString GetSelectedVariant () const;

		QString ReformatTitle ();

		void ReinitEntry ();

		bool eventFilter (QObject*, QEvent*);
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

		void handleEditScroll (int);
		void messageSend ();
		void on_MsgEdit__textChanged ();
		void on_SubjectButton__toggled (bool);
		void on_SubjChange__released ();
		void on_View__loadFinished (bool);
		void handleHistoryBack ();
		void handleRichEditorToggled ();
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
		void handleFileOffered (QObject*);
		void handleFileNoLongerOffered (QObject*);
		void handleOfferActionTriggered ();
		void handleEntryMessage (QObject*);
		void handleVariantsChanged (QStringList);
		void handleNameChanged (const QString& name);
		void handleStatusChanged (const EntryStatus&, const QString&);
		void handleChatPartStateChanged (const ChatPartState&, const QString&);
		void handleViewLinkClicked (QUrl, bool);
		void handleHistoryUp ();
		void handleHistoryDown ();

		void handleGotLastMessages (QObject*, const QList<QObject*>&);

		void handleSendButtonVisible ();
		void handleMinLinesHeightChanged ();
		void handleRichFormatterPosition ();

		void handleAccountStyleChanged (IAccount*);

		void performJS (const QString&);
	private:
		template<typename T>
		T* GetEntry () const;
		void BuildBasicActions ();
		void ReinitAvatar ();
		void CheckMUC ();
		void HandleMUC ();

		void InitExtraActions ();
		void AddManagedActions (bool first);

		void InitMsgEdit ();
		void RegisterSettings ();

		void RequestLogs (int);

		void UpdateTextHeight ();

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
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void needToClose (ChatTab*);
		void entryMadeCurrent (QObject*);
		void entryLostCurrent (QObject*);

		void tabRecoverDataChanged ();

		void composingTextChanged (const QString&);
		void currentVariantChanged (const QString&);

		// Hooks
		void hookChatTabCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebEngineView *webView);
		void hookGonnaAppendMsg (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookMadeCurrent (LC::IHookProxy_ptr proxy,
				QObject *chatTab);
		void hookMessageSendRequested (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
		void hookThemeReloaded (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QWebEngineView *view,
				QObject *entry);
	};

	typedef QPointer<ChatTab> ChatTab_ptr;
}
}
