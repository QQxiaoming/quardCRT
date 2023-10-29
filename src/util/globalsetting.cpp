#include <QApplication>
#include "globalsetting.h"

GlobalSetting::GlobalSetting(QObject *parent)
    : QSettings(QSettings::IniFormat, QSettings::UserScope,
        QApplication::applicationName(),QApplication::applicationName(),parent)
{
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
void GlobalSetting::setValue(QAnyStringView key, const QVariant &value)
#else
void GlobalSetting::setValue(const QString &key, const QVariant &value)
#endif
{
    QSettings::setValue(key, value);
    if(realtime)
        sync();
}

void GlobalSetting::setRealtimeSave(bool realtime)
{
    this->realtime = realtime;
}
