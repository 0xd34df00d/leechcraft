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

#pragma once

#include <QObject>
#include <QUrl>
#include <QIcon>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/iquarkcomponentprovider.h>
#include "manifest.h"

class QTranslator;

namespace LeechCraft
{
namespace SB2
{
	class ViewManager;
	class QuarkSettingsManager;

	class QuarkManager : public QObject
	{
		ViewManager * const ViewMgr_;
		ICoreProxy_ptr Proxy_;

		const QuarkComponent_ptr Component_;
		const QUrl URL_;

		Util::XmlSettingsDialog_ptr XSD_;
		QuarkSettingsManager *SettingsManager_;

		const std::shared_ptr<QTranslator> Translator_;

		const Manifest Manifest_;
	public:
		QuarkManager (QuarkComponent_ptr, ViewManager*, ICoreProxy_ptr);

		const Manifest& GetManifest () const;
		bool IsValidArea () const;

		bool HasSettings () const;
		Util::XmlSettingsDialog* GetXSD () const;
	private:
		QString GetSuffixedName (const QString&) const;
		std::shared_ptr<QTranslator> TryLoadTranslator () const;

		void CreateSettings ();
	};

	typedef std::shared_ptr<QuarkManager> QuarkManager_ptr;
}
}
