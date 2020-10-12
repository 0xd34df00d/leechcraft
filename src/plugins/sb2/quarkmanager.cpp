/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkmanager.h"
#include <QQmlEngine>
#include <QQmlContext>
#include <QQuickImageProvider>
#include <QStandardItem>
#include <QApplication>
#include <QTranslator>
#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <util/util.h>
#include <interfaces/iquarkcomponentprovider.h>
#include <interfaces/core/iiconthememanager.h>
#include "viewmanager.h"
#include "sbview.h"
#include "quarksettingsmanager.h"

namespace LC
{
namespace SB2
{
	namespace
	{
		class ImageProvProxy : public QQuickImageProvider
		{
			QQuickImageProvider *Wrapped_;
		public:
			ImageProvProxy (QQuickImageProvider *other)
			: QQuickImageProvider (other->imageType ())
			, Wrapped_ (other)
			{
			}

			QImage requestImage (const QString& id, QSize *size, const QSize& requestedSize)
			{
				return Wrapped_->requestImage (id, size, requestedSize);
			}

			QPixmap requestPixmap (const QString& id, QSize *size, const QSize& requestedSize)
			{
				return Wrapped_->requestPixmap (id, size, requestedSize);
			}
		};
	}

	QuarkManager::QuarkManager (QuarkComponent_ptr comp,
			ViewManager *manager, ICoreProxy_ptr proxy)
	: QObject (manager)
	, ViewMgr_ (manager)
	, Proxy_ (proxy)
	, Component_ (comp)
	, URL_ (comp->Url_)
	, SettingsManager_ (0)
	, Translator_ (TryLoadTranslator ())
	, Manifest_ (URL_.toLocalFile ())
	{
		if (!ViewMgr_)
			return;

		qDebug () << Q_FUNC_INFO << "adding" << comp->Url_;
		auto ctx = manager->GetView ()->rootContext ();
		for (const auto& pair : comp->StaticProps_)
			ctx->setContextProperty (pair.first, pair.second);
		for (const auto& pair : comp->DynamicProps_)
			ctx->setContextProperty (pair.first, pair.second);
		for (const auto& pair : comp->ContextProps_)
			ctx->setContextProperty (pair.first, pair.second.get ());

		auto engine = manager->GetView ()->engine ();
		for (const auto& pair : comp->ImageProviders_)
		{
			if (engine->imageProvider (pair.first))
				engine->removeImageProvider (pair.first);
			engine->addImageProvider (pair.first, new ImageProvProxy (pair.second));
		}

		CreateSettings ();
	}

	const Manifest& QuarkManager::GetManifest () const
	{
		return Manifest_;
	}

	bool QuarkManager::IsValidArea () const
	{
		const auto& areas = Manifest_.GetAreas ();
		return areas.isEmpty () || areas.contains ("panel");
	}

	bool QuarkManager::HasSettings () const
	{
		return SettingsManager_;
	}

	Util::XmlSettingsDialog* QuarkManager::GetXSD () const
	{
		return XSD_.get ();
	}

	QString QuarkManager::GetSuffixedName (const QString& suffix) const
	{
		if (!URL_.isLocalFile ())
			return {};

		const auto& localName = URL_.toLocalFile ();
		const auto& suffixed = localName + suffix;
		if (!QFile::exists (suffixed))
			return {};

		return suffixed;
	}

	std::shared_ptr<QTranslator> QuarkManager::TryLoadTranslator () const
	{
		if (!URL_.isLocalFile ())
			return {};

		auto dir = QFileInfo { URL_.toLocalFile () }.dir ();
		if (!dir.cd ("ts"))
			return {};

		const auto& locale = Util::GetLocaleName ();
		const auto& localeLang = locale.section ('_', 0, 0);

		const QStringList filters { "*_" + locale + ".qm", "*_" + localeLang + ".qm" };
		const auto& files = dir.entryList (filters, QDir::Files);
		if (files.isEmpty ())
			return {};

		std::shared_ptr<QTranslator> result
		{
			new QTranslator,
			[] (QTranslator *tr)
			{
				QApplication::removeTranslator (tr);
				delete tr;
			}
		};
		const auto& filename = dir.filePath (files.value (0));
		if (!result->load (filename))
			return {};

		QApplication::installTranslator (result.get ());

		return result;
	}

	void QuarkManager::CreateSettings ()
	{
		const auto& settingsName = GetSuffixedName (".settings");
		if (settingsName.isEmpty ())
			return;

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsManager_ = new QuarkSettingsManager (URL_, ViewMgr_->GetView ()->rootContext ());
		XSD_->RegisterObject (SettingsManager_, settingsName);
	}
}
}
