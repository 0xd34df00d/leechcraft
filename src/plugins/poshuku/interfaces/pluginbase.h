/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include <stdexcept>
#include <QObject>
#include <qwebpage.h>
#include <qwebframe.h>
#include "iwebplugin.h"

class QNetworkRequest;
class QWebView;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class IProxyObject;

			/** @brief Base class for all the plugins.
			 *
			 * Provides some kind of interface for communication with
			 * plugins.
			 *
			 * Functions either don't return anything, or return a boolean
			 * value (true means "stop processing", false means "continue"),
			 * or return some custom value. In the later case returning
			 * something means "stop processing and use this value instead
			 * of default", throwing any exception means "continue".
			 */
			class PluginBase
			{
			public:
				/** @brief Initializes the plugin.
				 *
				 * Initializes the plugin with the given proxy object.
				 * Through the proxy object plugin can access and manipulate
				 * LeechCraft::Poshuku's internals.
				 *
				 * @param[in] proxy Pointer to the proxy object.
				 */
				virtual void Init (IProxyObject *proxy) = 0;

				virtual ~PluginBase ()
				{
				}

				/** See the official Qt docs for the
				 * QWebPage::menuBarVisibilityChangeRequested() signal.
				 */
				virtual bool HandleMenuBarVisibilityChangeRequested (QWebPage*, bool)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::microFocusChanged() signal.
				 */
				virtual bool HandleMicroFocusChanged (QWebPage*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::repaintRequested() signal.
				 */
				virtual bool HandleRepaintRequested (QWebPage*, const QRect&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::restoreFrameStateRequested() signal.
				 */
				virtual bool HandleRestoreFrameStateRequested (QWebPage*, QWebFrame*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::saveFrameStateRequested() signal.
				 */
				virtual bool HandleSaveFrameStateRequested (QWebPage*, QWebFrame*, QWebHistoryItem*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::scrollRequested() signal.
				 */
				virtual bool HandleScrollRequested (QWebPage*, int, int, const QRect&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::selectionChanged() signal.
				 */
				virtual bool HandleSelectionChanged (QWebPage*)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::statusBarVisibilityChangeRequested() signal.
				 */
				virtual bool HandleStatusBarVisibilityChangeRequested (QWebPage*, bool)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::toolBarVisibilityChangeRequested() signal.
				 */
				virtual bool HandleToolBarVisibilityChangeRequested (QWebPage*, bool)
				{
					return false;
				}

				/** See the official Qt docs for the QWebPage::chooseFile().
				 */
				virtual QString OnChooseFile (QWebPage*, QWebFrame*, const QString&)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the QWebPage::createWindow().
				 */
				virtual QWebPage* OnCreateWindow (QWebPage*, QWebPage::WebWindowType)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptAlert().
				 */
				virtual bool OnJavaScriptAlert (QWebPage*, QWebFrame*, const QString&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptConfirm().
				 */
				virtual bool OnJavaScriptConfirm (QWebPage*, QWebFrame*, const QString&)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptConsoleMessage().
				 */
				virtual bool OnJavaScriptConsoleMessage (QWebPage*, const QString&,
						int, const QString&)
				{
					return false;
				}

				/** See the official Qt docs for the
				 * QWebPage::javaScriptPrompt().
				 */
				virtual bool OnJavaScriptPrompt (QWebPage*, QWebFrame*, const QString&,
						const QString&, QString*)
				{
					throw std::runtime_error ("We don't handle it by default.");
				}
			};

			typedef PluginBase *PluginBase_ptr;
		};
	};
};

Q_DECLARE_INTERFACE (LeechCraft::Plugins::Poshuku::PluginBase,
		"org.Deviant.LeechCraft.Plugins.Poshuku.PluginBase/1.0");

#endif

