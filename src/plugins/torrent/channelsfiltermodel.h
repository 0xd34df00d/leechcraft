#ifndef CHANNELSFILTERMODEL_H
#define CHANNELSFILTERMODEL_H
#include <QSortFilterProxyModel>

class ChannelsFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

    bool NormalMode_;
public:
    ChannelsFilterModel (QObject *parent = 0);
public slots:
	void setTagsMode ();
    void setNormalMode ();
protected:
    virtual bool filterAcceptsRow (int, const QModelIndex&) const;
};

#endif

