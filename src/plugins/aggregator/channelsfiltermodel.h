#ifndef CHANNELSFILTERMODEL_H
#define CHANNELSFILTERMODEL_H
#include <QSortFilterProxyModel>

class ChannelsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    ChannelsFilterModel (QObject *parent = 0);
protected:
    virtual bool filterAcceptsRow (int, const QModelIndex&) const;
};

#endif

