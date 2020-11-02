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

namespace LC::SB2
{
	namespace
	{
		class ImageProvProxy final : public QQuickImageProvider
		{
			QQuickImageProvider *Wrapped_;
		public:
			explicit ImageProvProxy (QQuickImageProvider *other)
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

	QuarkManager::QuarkManager (QuarkComponent_ptr comp, ViewManager *manager)
	: QObject (manager)
	, ViewMgr_ (manager)
	, Component_ (std::move (comp))
	, URL_ (comp->Url_)
	, Translator_ (TryLoadTranslator ())
	, Manifest_ (URL_.toLocalFile ())
	{
		if (!ViewMgr_)
			return;

		qDebug () << Q_FUNC_INFO << "adding" << Component_->Url_;
		QVector<QQmlContext::PropertyPair> props;
		for (const auto& [name, value] : Component_->StaticProps_)
			props.push_back ({ name, value });
		for (const auto& [name, value] : Component_->DynamicProps_)
			props.push_back ({ name, QVariant::fromValue (value) });
		for (const auto& [name, value] : Component_->ContextProps_)
			props.push_back ({ name, QVariant::fromValue (value.get ()) });
		manager->GetView ()->rootContext ()->setContextProperties (props);

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
		return areas.isEmpty () || areas.contains (QStringLiteral ("panel"));
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
		if (!dir.cd (QStringLiteral ("ts")))
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
		const auto& settingsName = GetSuffixedName (QStringLiteral (".settings"));
		if (settingsName.isEmpty ())
			return;

		XSD_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsManager_ = new QuarkSettingsManager (URL_, ViewMgr_->GetView ()->rootContext ());
		XSD_->RegisterObject (SettingsManager_, settingsName);
	}
}
