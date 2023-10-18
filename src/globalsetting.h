#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <QSettings>

class GlobalSetting : public QSettings
{
    Q_OBJECT
public:
    explicit GlobalSetting(QObject *parent = nullptr);
};

#endif // GLOBALSETTING_H
