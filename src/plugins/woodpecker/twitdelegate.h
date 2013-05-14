#include <QPainter>
#include <QAbstractItemDelegate>
#include <QRect>
#include <memory>
#include <interfaces/structures.h>

namespace LeechCraft
{
namespace Woodpecker
{
class TwitDelegate : public QAbstractItemDelegate
{
private:
  QObject* m_parent;
  
public:
  TwitDelegate(QObject *parent = 0);
  
  void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
  QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
  virtual ~TwitDelegate();
  QObject* parent();
  
signals:
  void gotEntity (const LeechCraft::Entity&);
  
};
}
}