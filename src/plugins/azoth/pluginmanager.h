/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINMANAGER_H
#define PLUGINS_AZOTH_PLUGINMANAGER_H
#include <QString>
#include <QDateTime>
#include <util/xpc/basehookinterconnector.h>
#include <interfaces/core/ihookproxy.h>

class QDateTime;
class QObject;
class QWebView;

namespace LC
{
namespace Azoth
{
	class PluginManager : public Util::BaseHookInterconnector
	{
		Q_OBJECT
	public:
		PluginManager (QObject* = 0);
	signals:
		void hookAddingCLEntryBegin (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookAddingCLEntryEnd (LC::IHookProxy_ptr proxy,
				QObject *entry);
		void hookChatTabCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				QWebView *webView);

		void hookCollectContactIcons (LC::IHookProxy_ptr, QObject*, QList<QIcon>&) const;

		void hookDnDEntry2Entry (LC::IHookProxy_ptr proxy,
				QObject *source,
				QObject *target);

		/** @brief Hook for adjusting where CL entry actions appear.
		 *
		 * This hook is called to determine where the given action for
		 * the given entry should be shown. By default, it is only shown
		 * in the contact list context menu. This hook is only called
		 * for actions that were created as a result of the
		 * hookEntryActionsRequested() hook.
		 *
		 * The handler of this hook should append the string IDs of the
		 * corresponding places to the return value of the proxy object,
		 * so an example implementation for inserting an action into
		 * contact list menu and tab's context menu would look like:
		 * @code
		 * QStringList ours;
		 * ours << "contactListContextMenu"
		 * 	<< "tabContextMenu";
		 * proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
		 * @endcode
		 *
		 * The following IDs are possible:
		 * - contactListContextMenu for contact list context menu (this
		 *   is the default option if the return value is unmodified).
		 * - tabContextMenu for tab's context menu.
		 * - applicationMenu for the menu item in the application's main
		 *   menu.
		 * - toolbar for the toolbar in the entry chat window.
		 *
		 * Please note that this hook would be called on each plugin
		 * that exposes it, so each plugin would have it called for each
		 * action created in hookEntryActionsRequested(), for all
		 * plugins, not only their own ones.
		 *
		 * @param[out] proxy The proxy object.
		 * @param[out] action The previously created action.
		 * @param[out] entry The entry for which action is queried.
		 *
		 * @sa hookEntryActionsRequested()
		 */
		void hookEntryActionAreasRequested (LC::IHookProxy_ptr proxy,
				QObject *action,
				QObject *entry);

		void hookEntryActionsRemoved (LC::IHookProxy_ptr proxy,
				QObject *entry);

		/** @brief Hook for adding actions for contact list entries.
		 *
		 * This hook is called for adding new CL entry-related actions
		 * could be.
		 *
		 * The passed entry object implements ICLEntry and represents
		 * the entry for which the action could be created. Of course,
		 * it's OK to not create actions for various entries, for
		 * example, after querying their type, state, etc.
		 *
		 * The exact places where the action would appear is later
		 * adjusted inside the hookEntryActionAreasRequested() hook.
		 *
		 * The handler of this hook should append the new actions to the
		 * proxy's return value, which is actually a list of QObjects.
		 * So, a typical hook would look like:
		 * @code
		 * QAction *action = new QAction (tr ("Some action"), entry);
		 * QList<QVariant> list = proxy->GetReturnValue ().toList ();
		 * list << QVariant::fromValue<QObject*> (action);
		 * proxy->SetReturnValue (list);
		 * @endcode
		 *
		 * Please note that it's better to create actions as children of
		 * the entry (as in this example) so that they are automatically
		 * deleted when the entry is deleted.
		 *
		 * @param[out] proxy The proxy object.
		 * @param[out] entry The object implementing ICLEntry and
		 * representing the entry for which actions are to be created.
		 *
		 * @sa hookEntryActionAreasRequested()
		 */
		void hookEntryActionsRequested (LC::IHookProxy_ptr proxy,
				QObject *entry);

		void hookEntryStatusChanged (LC::IHookProxy_ptr proxy,
				QObject *entry,
				QString variant);

		void hookGonnaAppendMsg (LC::IHookProxy_ptr proxy,
				QObject *message);

		void hookGonnaHandleSmiles (LC::IHookProxy_ptr proxy,
				QString body,
				QString pack);

		void hookGotAuthRequest (LC::IHookProxy_ptr proxy,
				QObject *entry,
				QString msg);

		/** @brief Hook for handling incoming messages.
		 *
		 * This hook is called for handling incoming messages. The
		 * message object could be modified accordingly, if possible,
		 * and the result would be visible to the rest of Azoth.
		 *
		 * The message object, of course, implements IMessage.
		 *
		 * If the hook handler cancels default handler (by calling
		 * IHookProxy::CancelDefault on the proxy object), nothing would
		 * be done with the message: particularly, it won't be appended
		 * to the chat window and such.
		 *
		 * @param[out] proxy The proxy object.
		 * @param[out] message The message object implementing IMessage.
		 */
		void hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookGotMessage2 (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookFormatDateTime (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QDateTime dateTime,
				QObject *message);
		void hookFormatNickname (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QString nick,
				QObject *message);
		void hookFormatBodyBegin (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookFormatBodyEnd (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookIsHighlightMessage (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookMadeCurrent (LC::IHookProxy_ptr proxy,
				QObject *chatTab);
		void hookMessageSendRequested (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
		void hookMessageWillCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *entry,
				int type,
				QString variant);
		void hookMessageCreated (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QObject *message);
		void hookShouldCountUnread (LC::IHookProxy_ptr proxy,
				QObject *message);
		void hookThemeReloaded (LC::IHookProxy_ptr proxy,
				QObject *chatTab,
				QWebView *view,
				QObject *entry);

		/** @brief Hook for tooltip formatting.
		 *
		 * This hook is called while formatting tooltip for the contact
		 * list tree for the given entry, after general information
		 * about the entry has been formatted.
		 *
		 * The already-formatted string is contained as "tooltip" value
		 * in the proxy. The hook may change the string by updating this
		 * value.
		 *
		 * If the hook handler cancels default handler (by calling
		 * IHookProxy::CancelDefault on the proxy object), variants info
		 * won't be formatted, and the hook's "tooltip" value would be
		 * returned.
		 *
		 * @param[out] proxy Standard proxy object.
		 * @param[out] entry The entry for which the tooltip is being
		 * formatted.
		 * @param[in,out] proxy::"tooltip" The tooltip string.
		 *
		 * @sa IHookProxy
		 */
		void hookTooltipBeforeVariants (LC::IHookProxy_ptr proxy,
				QObject *entry);
	};
}
}

#endif
