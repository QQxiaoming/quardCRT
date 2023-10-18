#include <QApplication>
#include "globalsetting.h"

GlobalSetting::GlobalSetting(QObject *parent)
    : QSettings(QSettings::IniFormat, QSettings::UserScope,
        QApplication::applicationName(),QApplication::applicationName(),parent)
{

}
