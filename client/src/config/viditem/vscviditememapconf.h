#ifndef __VSC_VID_ITEM_EMAP_CONF_H__
#define __VSC_VID_ITEM_EMAP_CONF_H__

#include "common/vscviditeminf.h"

class VSCVidItemEmapConf : public QObject,public VSCVidItemInf
{
Q_OBJECT
public:
    VSCVidItemEmapConf(ClientFactory &pFactory, QTreeWidgetItem *parent);
    ~VSCVidItemEmapConf();
public:

};

#endif /* __VSC_VID_ITEM_EMAP_CONF_H__ */