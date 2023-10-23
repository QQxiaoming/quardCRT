#ifndef GLOBALSETTING_H
#define GLOBALSETTING_H

#include <QSettings>

class GlobalSetting : public QSettings
{
    Q_OBJECT
public:
    explicit GlobalSetting(QObject *parent = nullptr);

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    void setValue(QAnyStringView key, const QVariant &value);
#else
    void setValue(const QString &key, const QVariant &value);
#endif
    void setRealtimeSave(bool realtime);
    bool isRealtimeSave() const { return realtime;}

private:
    bool realtime = false;
};

#endif // GLOBALSETTING_H
