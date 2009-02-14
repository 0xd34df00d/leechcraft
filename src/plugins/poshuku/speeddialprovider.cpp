#include "speeddialprovider.h"
#include <QBuffer>
#include <QtDebug>
#include <QPainter>
#include <QWebPage>
#include <QWebFrame>
#include <QApplication>
#include "xmlsettingsmanager.h"
#include "core.h"

SpeedDialProvider::SpeedDialProvider ()
{
}

SpeedDialProvider& SpeedDialProvider::Instance ()
{
	static SpeedDialProvider sdp;
	return sdp;
}

QString SpeedDialProvider::GetHTML () const
{
	if (!XmlSettingsManager::Instance ()->property ("SpeedDialEnabled").toBool ())
		return QString ();

	QString result = "<html><head><title>";
	result += tr ("Speed dial");
	result += "</title><body><table style='width:100%;height:100%;"
		"border-spacing:0pt;'>";

	result += GetTopHistory ();

	result += "</table></body></html>";

	return result;
}

QString SpeedDialProvider::GetTopHistory () const
{
	QString result;
	StorageBackend *storage = Core::Instance ().GetStorageBackend ();

	history_items_t items;
	storage->LoadResemblingHistory ("", items);
	if (!items.size ())
		return QString ();

	int rows = XmlSettingsManager::Instance ()->
		property ("SpeedDialRows").toInt ();
	int columns = XmlSettingsManager::Instance ()->
		property ("SpeedDialColumns").toInt ();

	size_t pos = 0;
	bool shouldBreak = false;

	for (int i = 0; i < rows; ++i)
	{
		result += "<tr>";
		for (int j = 0; j < columns; ++j)
		{
			result += "<td style='border:1px solid black;text-align:center'>";
			result += GetHTMLForItem (items.at (i * columns + j));
			result += "</td>";

			if (++pos == items.size ())
			{
				break;
				shouldBreak = true;
			}
		}

		result += "</tr>";

		if (shouldBreak)
			break;
	}

	return result;
}

QString SpeedDialProvider::GetHTMLForItem (const HistoryItem& hitem) const
{
	QString title = hitem.Title_;
	if (title.size () > 37)
	{
		title = title.left (37);
		title += "...";
	}
	QString result = "<a href='";
	result += hitem.URL_;
	result += "' target='_blank'>";

	StorageBackend *storage = Core::Instance ().GetStorageBackend ();

	Item item;
	item.URL_ = hitem.URL_;
	storage->GetThumbnail (item);

	int width = XmlSettingsManager::Instance ()->
		property ("SpeedDialThumbXResolution").toInt ();
	int height = XmlSettingsManager::Instance ()->
		property ("SpeedDialThumbYResolution").toInt ();

	bool shouldRegenerate = item.Thumb_.isEmpty () ||
		item.ResX_ < width ||
		item.ResY_ < height;
	if (shouldRegenerate)
	{
		Regenerate (hitem.URL_);
		result += tr ("<strong>Regenerating</strong> for %1...")
			.arg (title);
	}
	else
	{
		result += "<img src='data:image/png;base64,";
		QImage image = QImage::fromData (QByteArray::fromBase64 (item.Thumb_))
			.scaled (width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		QBuffer renderBuffer;
		image.save (&renderBuffer, "PNG", 50);

		result += renderBuffer.data ().toBase64 ();
		result += "' /><br />";
		result += title;
	}
	result += "</a>";

	return result;
}

void SpeedDialProvider::Regenerate (const QString& url) const
{
	if (RenderQueue_.contains (url))
		return;

	RenderQueue_ << url;

	QWebPage *page = new QWebPage ();
	page->setNetworkAccessManager (Core::Instance ().GetNetworkAccessManager ());
	page->setProperty ("URL", url);
	connect (page,
			SIGNAL (loadFinished (bool)),
			this,
			SLOT (handleFinished ()));
	page->mainFrame ()->load (QUrl (url));
}

void SpeedDialProvider::handleFinished ()
{
	QWebPage *page = qobject_cast<QWebPage*> (sender ());
	int width = XmlSettingsManager::Instance ()->
		property ("SpeedDialThumbXResolution").toInt ();
	int height = XmlSettingsManager::Instance ()->
		property ("SpeedDialThumbYResolution").toInt ();

	if (page->mainFrame ()->contentsSize ().isEmpty () ||
			page->property ("URL").toString ().isEmpty ())
		return;
	qDebug () << page->property ("URL");

	page->setViewportSize (page->mainFrame ()->contentsSize ());

	QImage image (page->viewportSize (), QImage::Format_ARGB32);
	QPainter painter (&image);
	int rWidth = page->viewportSize ().width (); 
	int rHeight = height * rWidth / width + 1;
	page->mainFrame ()->render (&painter, QRegion (0, 0, rWidth, rHeight));

	QApplication::processEvents ();

	painter.end ();

	QImage thumbnail = image
		.scaledToWidth (width, Qt::FastTransformation)
		.copy (0, 0, width, height);

	QApplication::processEvents ();

	QBuffer renderBuffer;
	thumbnail.save (&renderBuffer, "PNG", 50);

	SpeedDialProvider::Item item;
	item.URL_ = page->property ("URL").toString ();
	item.ResX_ = width;
	item.ResY_ = height;
	item.Thumb_ = renderBuffer.data ().toBase64 ();
	item.ShotDate_ = QDateTime::currentDateTime ();

	Core::Instance ().GetStorageBackend ()->SetThumbnail (item);
	RenderQueue_.removeAll (item.URL_);
	page->deleteLater ();

	emit newThumbAvailable ();
}

