/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/core/ihookproxy.h>
#include <interfaces/ispellcheckprovider.h>

class QWebView;
class QTranslator;

namespace LC
{
namespace Azoth
{
namespace Rosenthal
{
	class Highlighter;
	class Checker;

	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
	{
		Q_OBJECT
		Q_INTERFACES (IInfo IPlugin2)

		LC_PLUGIN_METADATA ("org.LeechCraft.Azoth.Rosenthal")

		ICoreProxy_ptr Proxy_;

		ISpellChecker_ptr Checker_;

		QList<Highlighter*> Highlighters_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;
		QSet<QByteArray> GetPluginClasses () const;
	protected:
		bool eventFilter (QObject*, QEvent*);
	private slots:
		void hookChatTabCreated (LC::IHookProxy_ptr,
				QObject*,
				QObject*,
				QWebView*);
		void handleCorrectionTriggered ();
		void handleHighlighterDestroyed ();
	};
}
}
}
