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

#include "pluginmanager.h"
#include <stdexcept>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <QtDebug>
#include "proxyobject.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			PluginManager::PluginManager (QObject *parent)
			: QObject (parent)
			, ProxyObject_ (new ProxyObject)
			{
			}
			
			void PluginManager::AddPlugin (QObject *plugin)
			{
				PluginBase_ptr base = qobject_cast<PluginBase_ptr> (plugin);
				if (!base)
				{
					qWarning () << Q_FUNC_INFO
						<< "passed plugin is not a valid Poshuku plugin"
						<< plugin;
					return;
				}
				base->Init (ProxyObject_.get ());
				Plugins_.push_back (base);
			}
			
#define SEQ_TRAVERSER(z,i,array) \
				BOOST_PP_COMMA_IF(i) BOOST_PP_SEQ_ELEM(i,array) param ## i
			
#define BE(Func,Params)																		\
				bool PluginManager::Func (																\
						BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Params), SEQ_TRAVERSER, Params)				\
						)																				\
				{																						\
					Q_FOREACH (PluginBase_ptr ptr, Plugins_)											\
						if (ptr->Func (BOOST_PP_ENUM_PARAMS (BOOST_PP_SEQ_SIZE (Params), param)))		\
							return true;																\
																										\
					return false; 																		\
				}
			
#define CE(Func,Params,R)																	\
				R PluginManager::Func (																	\
						BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Params), SEQ_TRAVERSER, Params)				\
						)																				\
				{																						\
					Q_FOREACH (PluginBase_ptr ptr, Plugins_)											\
						try																				\
						{																				\
							return ptr->Func (BOOST_PP_ENUM_PARAMS (BOOST_PP_SEQ_SIZE (Params), param));\
						}																				\
						catch (...)																		\
						{																				\
						}																				\
																										\
					throw std::runtime_error ("No plugins handled the input.");							\
				}
			
#define VE(Func,Params)																		\
				void PluginManager::Func (																\
						BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Params), SEQ_TRAVERSER, Params)				\
						)																				\
				{																						\
					Q_FOREACH (PluginBase_ptr ptr, Plugins_)											\
						ptr->Func (BOOST_PP_ENUM_PARAMS (BOOST_PP_SEQ_SIZE (Params), param));			\
				}
			
			VE (Init, (IProxyObject*));
			BE (HandleWebPluginFactoryReload, (QList<IWebPlugin*>&));
			BE (HandleBeginWebPageConstruction, (QWebPage*));
			BE (HandleEndWebPageConstruction, (QWebPage*));
			BE (HandleContentsChanged, (QWebPage*));
			BE (HandleDatabaseQuotaExceeded, (QWebPage*)(QWebFrame*)(QString));
			BE (HandleDownloadRequested, (QWebPage*)(const QNetworkRequest&));
			BE (HandleFrameCreated, (QWebPage*)(QWebFrame*));
			BE (HandleGeometryChangeRequested, (QWebPage*)(const QRect&));
			BE (HandleJavaScriptWindowObjectCleared, (QWebPage*)(QWebFrame*));
			BE (HandleLinkClicked, (QWebPage*)(const QUrl&));
			BE (HandleLinkHovered, (QWebPage*)(const QString&)(const QString&)(const QString&));
			BE (HandleLoadFinished, (QWebPage*)(bool));
			BE (HandleLoadProgress, (QWebPage*)(int));
			BE (HandleLoadStarted, (QWebPage*));
			BE (HandleMenuBarVisibilityChangeRequested, (QWebPage*)(bool));
			BE (HandleMicroFocusChanged, (QWebPage*));
			BE (HandlePrintRequested, (QWebPage*)(QWebFrame*));
			BE (HandleRepaintRequested, (QWebPage*)(const QRect&));
			BE (HandleRestoreFrameStateRequested, (QWebPage*)(QWebFrame*));
			BE (HandleSaveFrameStateRequested, (QWebPage*)(QWebFrame*)(QWebHistoryItem*));
			BE (HandleScrollRequested, (QWebPage*)(int)(int)(const QRect&));
			BE (HandleSelectionChanged, (QWebPage*));
			BE (HandleStatusBarMessage, (QWebPage*)(const QString&));
			BE (HandleStatusBarVisibilityChangeRequested, (QWebPage*)(bool));
			BE (HandleToolBarVisibilityChangeRequested, (QWebPage*)(bool));
			BE (HandleUnsupportedContent, (QWebPage*)(QNetworkReply*));
			BE (HandleWindowCloseRequested, (QWebPage*));
			BE (OnAcceptNavigationRequest,
					(QWebPage*)(QWebFrame*)(const QNetworkRequest&)(QWebPage::NavigationType));
			CE (OnChooseFile, (QWebPage*)(QWebFrame*)(const QString&), QString);
			CE (OnCreatePlugin,
					(QWebPage*)(const QString&)(const QUrl&)(const QStringList&)(const QStringList&),
					QObject*);
			CE (OnCreateWindow, (QWebPage*)(QWebPage::WebWindowType), QWebPage*);
			BE (OnJavaScriptAlert, (QWebPage*)(QWebFrame*)(const QString&));
			CE (OnJavaScriptConfirm, (QWebPage*)(QWebFrame*)(const QString&), bool);
			BE (OnJavaScriptConsoleMessage, (QWebPage*)(const QString&)(int)(const QString&));
			CE (OnJavaScriptPrompt, (QWebPage*)(QWebFrame*)(const QString&)(const QString&)(QString*),
					bool);
			CE (OnUserAgentForUrl, (const QWebPage*)(const QUrl&), QString);
			BE (OnWebViewCtxMenu,
					(QWebView*)(QContextMenuEvent*)(const QWebHitTestResult&)(QMenu*)(WebViewCtxMenuStage));
		};
	};
};

