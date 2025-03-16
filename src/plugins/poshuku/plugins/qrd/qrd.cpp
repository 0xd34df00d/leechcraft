/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "qrd.h"
#include <QMessageBox>
#include <QApplication>
#include <QLabel>
#include <QMenu>
#include <QKeyEvent>
#include <qrencode.h>
#include <util/util.h>
#include <util/gui/geometry.h>
#include <util/sll/lambdaeventfilter.h>

namespace LC::Poshuku::QRd
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Util::InstallTranslator (QStringLiteral ("poshuku_qrd"));
	}

	void Plugin::SecondInit ()
	{
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.QRd";
	}

	QString Plugin::GetName () const
	{
		return QStringLiteral ("Poshuku QRd");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Shows the QR code with the URL of an opened web page.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Poshuku.Plugins/1.0" };
	}

	void Plugin::hookWebViewContextMenu (const IHookProxy_ptr&,
			IWebView *view, const ContextMenuInfo& info,
			QMenu *menu, WebViewCtxMenuStage menuBuildStage)
	{
		if (menuBuildStage != WVSAfterFinish)
			return;

		const auto& url = view->GetUrl ();
		if (!url.isEmpty ())
		{
			const auto act = menu->addAction (tr ("Generate QR code..."),
					this, &Plugin::GenQR);
			act->setProperty ("Poshuku/QRd/URL", url);
		}

		if (!info.LinkUrl_.isEmpty ())
		{
			const auto act = menu->addAction (tr ("Generate QR code for the link..."),
					this, &Plugin::GenQR);
			act->setProperty ("Poshuku/QRd/URL", info.LinkUrl_);
		}
	}

	void Plugin::GenQR ()
	{
		const auto& url = sender ()->property ("Poshuku/QRd/URL").toUrl ();

		const auto& encoded = url.toEncoded ();
		const std::unique_ptr<QRcode, decltype (&QRcode_free)> code
		{
			QRcode_encodeString (encoded.constData (),
					0, QR_ECLEVEL_H, QR_MODE_8, true),
			&QRcode_free
		};

		if (!code)
		{
			QMessageBox::critical (nullptr,
					QStringLiteral ("LeechCraft"),
					tr ("Failed to generate QR code for the page."));
			return;
		}

		const auto whiteBorder = 4;

		const auto width = code->width;
		const auto fullWidth = width + 2 * whiteBorder;
		QImage image { fullWidth, fullWidth, QImage::Format_Mono };
		image.setColor (0, QColor { Qt::white }.rgb ());
		image.setColor (1, QColor { Qt::black }.rgb ());
		image.fill (0);
		for (int y = 0; y < width; ++y)
			for (int x = 0; x < width; ++x)
				image.setPixel (x + whiteBorder, y + whiteBorder,
						code->data [y * width + x] & 0x01);

		const auto& geom = Util::AvailableGeometry (QCursor::pos ());
		const auto dim = std::min (geom.width (), geom.height ());
		if (dim < fullWidth)
		{
			QMessageBox::critical (nullptr,
					QStringLiteral ("LeechCraft"),
					tr ("Sorry, but the QR code is bigger than your display."));
			return;
		}

		const auto maxRatio = 2.0 / 3;
		auto scale = (fullWidth < maxRatio * dim) ?
				(maxRatio * dim / fullWidth) :
				(static_cast<float> (dim) / static_cast<float> (fullWidth));
		if (scale > 1)
		{
			const auto newDim = static_cast<int> (fullWidth * scale);
			image = image.scaled (newDim, newDim, Qt::KeepAspectRatio, Qt::FastTransformation);
		}

		auto label = new QLabel;
		label->setWindowTitle (tr ("QR code for %1")
				.arg (QString::fromUtf8 (encoded)));
		label->setAttribute (Qt::WA_DeleteOnClose);
		label->setPixmap (QPixmap::fromImage (image));
		label->show ();
		label->installEventFilter (Util::MakeLambdaEventFilter ([label] (QKeyEvent *ev)
				{
					if (ev->type () == QEvent::KeyRelease && ev->key () == Qt::Key_Escape)
						label->deleteLater ();

					return false;
				},
				*label));
	}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_qrd, LC::Poshuku::QRd::Plugin);
