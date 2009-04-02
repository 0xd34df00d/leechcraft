#include "pluginmanager.h"
#include <stdexcept>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

using namespace LeechCraft::Poshuku;

PluginManager::PluginManager (QObject *parent)
{
}

void PluginManager::AddPlugin (QObject *plugin)
{
	PluginBase_ptr base = qobject_cast<PluginBase_ptr> (plugin);
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


#define PARSEQ (IProxyObject*)
VE (Init, PARSEQ);
#undef PARSEQ

#define PARSEQ (QWebPage*)(const QNetworkRequest&)
BE (OnHandleDownloadRequested, PARSEQ);
#undef PARSEQ

#define PARSEQ (QWebPage*)(QNetworkReply*)
BE (OnGotUnsupportedContent, PARSEQ);
#undef PARSEQ

#define PARSEQ (QWebPage*)(QWebFrame*)(const QNetworkRequest&)(QWebPage::NavigationType)
BE (OnAcceptNavigationRequest, PARSEQ);
#undef PARSEQ

#define PARSEQ (QWebPage*)(QWebFrame*)(const QString&)
CE (OnChooseFile, PARSEQ, QString);
#undef PARSEQ

#define PARSEQ (QWebPage*)(const QString&)(const QUrl&)(const QStringList&)(const QStringList&)
CE (OnCreatePlugin, PARSEQ, QObject*);
#undef PARSEQ

#define PARSEQ (QWebPage*)(QWebPage::WebWindowType)
CE (OnCreateWindow, PARSEQ, QWebPage*);
#undef PARSEQ

#define PARSEQ (QWebPage*)(QWebFrame*)(const QString&)
BE (OnJavaScriptAlert, PARSEQ);
#undef PARSEQ

#define PARSEQ (QWebPage*)(QWebFrame*)(const QString&)
CE (OnJavaScriptConfirm, PARSEQ, bool);
#undef PARSEQ

#define PARSEQ (QWebPage*)(const QString&)(int)(const QString&)
BE (OnJavaScriptConsoleMessage, PARSEQ);
#undef PARSEQ

#define PARSEQ (QWebPage*)(QWebFrame*)(const QString&)(const QString&)(QString*)
CE (OnJavaScriptPrompt, PARSEQ, bool);
#undef PARSEQ

#define PARSEQ (const QWebPage*)(const QUrl&)
CE (OnUserAgentForUrl, PARSEQ, QString);
#undef PARSEQ

