#include "twitdelegate.h"
#include "core.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <QDebug>
#include <QUrl>
#include <qpushbutton.h>
#include <qapplication.h>
#include <QMouseEvent>
#include <interfaces/structures.h>
#include <util/util.h>
#include <QTextDocument>
#include <QRectF>

namespace LeechCraft
{
namespace Woodpecker
{
TwitDelegate::TwitDelegate(QObject *parent)
{
  
}

void TwitDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
  QRect r = option.rect;
  
  //Color: #C4C4C4
  QPen linePen(QColor::fromRgb(211,211,211), 1, Qt::SolidLine);
  
  //Color: #005A83
  QPen lineMarkedPen(QColor::fromRgb(0,90,131), 1, Qt::SolidLine);
  
  //Color: #333
  QPen fontPen(QColor::fromRgb(51,51,51), 1, Qt::SolidLine);
  
  //Color: #Link
  QPen linkFontPen(QColor::fromRgb(51,51,255), 1, Qt::SolidLine);
  
  //Color: #fff
  QPen fontMarkedPen(Qt::white, 1, Qt::SolidLine);
  
  QFont mainFont( "Dejavu Sans", 10, QFont::Normal );
  
  if(option.state & QStyle::State_Selected){
    QLinearGradient gradientSelected(r.left(),r.top(),r.left(),r.height()+r.top());
    gradientSelected.setColorAt(0.0, QColor::fromRgb(119,213,247));
    gradientSelected.setColorAt(0.9, QColor::fromRgb(27,134,183));
    gradientSelected.setColorAt(1.0, QColor::fromRgb(0,120,174));
    painter->setBrush(gradientSelected);
    painter->drawRect(r);
    
    // Border
    painter->setPen(lineMarkedPen);
    painter->drawLine(r.topLeft(),r.topRight());
    painter->drawLine(r.topRight(),r.bottomRight());
    painter->drawLine(r.bottomLeft(),r.bottomRight());
    painter->drawLine(r.topLeft(),r.bottomLeft());
    
    painter->setPen(fontMarkedPen);
    
  } else {
    // Background
    // Alternating colors
    painter->setBrush( (index.row() % 2) ? Qt::white : QColor(252,252,252) );
    painter->drawRect(r);
    
    // border
    painter->setPen(linePen);
    painter->drawLine(r.topLeft(),r.topRight());
    painter->drawLine(r.topRight(),r.bottomRight());
    painter->drawLine(r.bottomLeft(),r.bottomRight());
    painter->drawLine(r.topLeft(),r.bottomLeft());
    
    painter->setPen(fontPen);
  }
  
  // Get title, description and icon
  QIcon ic = QIcon(qvariant_cast<QPixmap>(index.data(Qt::DecorationRole)));
  QString text = index.data(Qt::DisplayRole).toString();
  qulonglong id = index.data(Qt::UserRole).toULongLong();
  QString author = index.data(Qt::UserRole + 1).toString();
  QString time = index.data(Qt::UserRole + 2).toString();
  
  int imageSpace = 50;
  if (!ic.isNull()) {
    // Icon
    r = option.rect.adjusted(5, 10, -10, -10);
    ic.paint(painter, r, Qt::AlignVCenter | Qt::AlignLeft);
    imageSpace = 60;
	if ( tweet_links.find(id) == tweet_links.end() )
	{
		QRect r(10, 10, 20, 50);
		tweet_links.insert(std::make_pair (id, std::make_pair(r, QUrl("http://ya.ru"))));
	};
  }
  
  // Text
  r = option.rect.adjusted(imageSpace, 4, -10, -22);
  painter->setFont( mainFont );
  QTextDocument doc;
  doc.setHtml( text );
  doc.setTextWidth( r.width() );
  painter->save();
  painter->translate( r.left(), r.top() );
  doc.drawContents( painter, r.translated( -r.topLeft()) );
  painter->restore();
  
  // Author
  r = option.rect.adjusted(imageSpace, 30, -10, 0);
  auto author_rect = std::unique_ptr<QRect>( new QRect(r.left(), r.bottom() - painter->fontMetrics().height() - 8, painter->fontMetrics().width(author), r.height()));
  painter->setPen( linkFontPen );
  painter->setFont( mainFont );
  painter->drawText(*(author_rect), Qt::AlignLeft, author, &r);
  painter->setPen(fontPen);
  
  // Time
  r = option.rect.adjusted(imageSpace, 30, -10, 0);
  painter->setFont( mainFont );
  painter->drawText(r.right() - painter->fontMetrics().width(time), r.bottom() - painter->fontMetrics().height() - 8, r.width(), r.height(), Qt::AlignLeft, time, &r);
  painter->setPen(linePen);
  
  // Links
  QRegExp rx("\\s((http|https)://[a-z0-9]+([-.]{1}[a-z0-9]+)*.[a-z]{2,5}(([0-9]{1,5})?/?.*))(\\s|,|$)");
  rx.setMinimal(true);
  
  qDebug() << "Parsing links for tweet " << id;
  
  if (rx.indexIn(text) != -1) {
	  for (auto link : rx.capturedTexts())
	  {
		  qDebug() << link;
	  }
	  qDebug() << "The link: " << rx.capturedTexts()[1];
  }
}

QSize TwitDelegate::sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const{
  return QSize(200, 60); // very dumb value
}

TwitDelegate::~TwitDelegate()
{
}

bool TwitDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
	if (event->type() == QEvent::MouseButtonRelease) {
		qulonglong id = index.data(Qt::UserRole).toULongLong();
		qDebug() << "Mouse button released in Twit " << id << "!";
		QMouseEvent *me = (QMouseEvent*)event;
             if (me) {
				auto position = (me->pos() - option.rect.topLeft());
				qDebug() << "Coordinates: " << position.x() << ", " << position.y();
				if ( tweet_links.find(id) != tweet_links.end() )
					qDebug() << "And it contains link at "<< tweet_links[id].first.topLeft() << " - " << tweet_links[id].first.bottomRight();
			 }
		
	}
    return QAbstractItemDelegate::editorEvent(event, model, option, index);
}


}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
