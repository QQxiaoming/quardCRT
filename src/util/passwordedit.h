#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include "qfonticon.h"

#include <QLineEdit>
#include <QAction>
#include <QIcon>

class PasswordEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit PasswordEdit(bool show_visibility = true, QWidget *parent = nullptr) 
        : QLineEdit(parent) {
        if (show_visibility) {
            // Add the password hide/shown toggle at the end of the edit box.
            togglepasswordAction = addAction(QFontIcon::icon(QChar(0xf06e)), QLineEdit::TrailingPosition);
            connect(togglepasswordAction, &QAction::triggered, this, &PasswordEdit::on_toggle_password_Action);
        }
        password_shown = false;
        setEchoMode(QLineEdit::Password);
    }

private slots:
    void on_toggle_password_Action() {
        if (!password_shown) {
            setEchoMode(QLineEdit::Normal);
            password_shown = true;
            togglepasswordAction->setIcon(QFontIcon::icon(QChar(0xf070)));
        } else {
            setEchoMode(QLineEdit::Password);
            password_shown = false;
            togglepasswordAction->setIcon(QFontIcon::icon(QChar(0xf06e)));
        }
    }
    
private:
    QAction *togglepasswordAction;
    bool password_shown;
};
    

#endif // PASSWORDEDIT_H
