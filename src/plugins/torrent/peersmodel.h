#ifndef PEERSMODEL_H
#define PEERSMODEL_H
#include <QAbstractItemModel>
#include <QStringList>
#include <QList>
#include "peerinfo.h"

class PeersModel : public QAbstractItemModel
{
    Q_OBJECT

    friend class Core;

    QStringList Headers_;
    QList<PeerInfo> Peers_;
    int CurrentTorrent_;
public:
    enum { SortRole = 45 };
    PeersModel (QObject *parent = 0);
    virtual ~PeersModel ();

    virtual int columnCount (const QModelIndex&) const;
    virtual QVariant data (const QModelIndex&, int role = Qt::DisplayRole) const;
    virtual Qt::ItemFlags flags (const QModelIndex&) const;
    virtual bool hasChildren (const QModelIndex&) const;
    virtual QVariant headerData (int, Qt::Orientation, int role = Qt::DisplayRole) const;
    virtual QModelIndex index (int, int, const QModelIndex& parent = QModelIndex ()) const;
    virtual QModelIndex parent (const QModelIndex&) const;
    virtual int rowCount (const QModelIndex& parent = QModelIndex ()) const;
protected:
    void Clear ();
    void Update (const QList<PeerInfo>&, int);
};

#endif

