/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "document.h"
#include <QFile>
#include <QDomDocument>
#include <QTextDocument>
#include <QApplication>
#include <QPalette>
#include <QtDebug>
#include <util/sll/either.h>
#include "fb2converter.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Monocle
{
namespace FXB
{
	Document::Document (const QString& filename, QObject *plugin)
	: DocURL_ (QUrl::fromLocalFile (filename))
	, Plugin_ (plugin)
	{
		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		QDomDocument fb2;
		if (!fb2.setContent (file.readAll (), true))
		{
			qWarning () << Q_FUNC_INFO
					<< "malformed XML in"
					<< filename;
			return;
		}

		FB2Converter::Config cfg {};
		cfg.DefaultFont_ = XmlSettingsManager::Instance ().property ("DefaultFont").value<QFont> ();
		cfg.PageSize_ = {
				XmlSettingsManager::Instance ().property ("PageWidth").toInt (),
				XmlSettingsManager::Instance ().property ("PageHeight").toInt ()
			};
		cfg.Margins_ = {
				XmlSettingsManager::Instance ().property ("LeftMargin").toInt (),
				XmlSettingsManager::Instance ().property ("TopMargin").toInt (),
				XmlSettingsManager::Instance ().property ("RightMargin").toInt (),
				XmlSettingsManager::Instance ().property ("BottomMargin").toInt ()
			};
		cfg.BackgroundColor_ = qApp->palette ().color (QPalette::Base);
		cfg.LinkColor_ = qApp->palette ().color (QPalette::Link);

		Util::Visit (FB2Converter { this, fb2 }.GetResult (),
				[this] (FB2Converter::ConvertedDocument&& result)
				{
					SetDocument (std::move (result.Doc_), result.Links_);
					Info_ = result.Info_;
					TOC_ = result.TOC_;
				},
				[] (const FB2Converter::Error_t&) {});
	}

	QObject* Document::GetBackendPlugin () const
	{
		return Plugin_;
	}

	QObject* Document::GetQObject ()
	{
		return this;
	}

	DocumentInfo Document::GetDocumentInfo () const
	{
		return Info_;
	}

	QUrl Document::GetDocURL () const
	{
		return DocURL_;
	}

	TOCEntryLevel_t Document::GetTOC ()
	{
		return TOC_;
	}

	void Document::RequestNavigation (int page)
	{
		emit navigateRequested ({}, { page, { 0, 0.4 } });
	}
}
}
}
