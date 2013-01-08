#include <QPainter>
#include <QAbstractItemDelegate>

namespace LeechCraft
{
namespace Woodpecker
{
class TwitDelegate : public QAbstractItemDelegate
{
public:
  TwitDelegate(QObject *parent = 0);
  
  void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
  QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
  
  virtual ~TwitDelegate();
  
};
}
}