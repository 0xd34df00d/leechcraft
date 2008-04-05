#ifndef PLUGINLIST_H
#define PLUGINLIST_H
#include <QListWidget>

class PluginList : public QListWidget
{
    Q_OBJECT
public:
    PluginList (QWidget *parent = 0);
};

#endif

