/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2023 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <QLabel>
#include <QListView>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QStringListModel>
#include <QMenu>
#include <QDebug>
#include "filedialog.h"
#include "ui_filedialog.h"

FileDialog::FileDialog(QWidget *parent, const QString &caption,
                        const QString &dir, const QString &filter)
    : QDialog(parent)
    , ui(new Ui::FileDialog)
{
    ui->setupUi(this);
    
    m_fileDialog = new SFDFileDialog(this, caption, dir, filter);
    m_fileDialog->setWindowFlags(Qt::Widget);
    m_fileDialog->setSizeGripEnabled(false);
    m_fileDialog->findChild<QDialogButtonBox*>("buttonBox")->hide();

    m_listView = new QListView(this);
    m_listView->setMinimumHeight(100);
    m_listView->setModel(new QStringListModel(this));
    m_listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_listView, &QListView::customContextMenuRequested, this, [this](const QPoint &pos) {
        if (m_listView->currentIndex().row() < 0) {
            return;
        }
        QMenu menu;
        menu.addAction(tr("Remove"), [this]() {
            static_cast<QStringListModel*>(m_listView->model())->removeRow(m_listView->currentIndex().row());
        });
        menu.exec(m_listView->mapToGlobal(pos)+QPoint(5,5));
    });

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);

    this->layout()->addWidget(m_fileDialog);
    this->layout()->addWidget(new QLabel(tr("Files to send:"),this));
    this->layout()->addWidget(m_listView);
    this->layout()->addWidget(m_buttonBox);

    setWindowTitle(caption);

    connect(m_fileDialog, &SFDFileDialog::acceptFile, this, [this](const QString &file) {
        if (file.isEmpty()) {
            return;
        }
        QStringList files = static_cast<QStringListModel*>(m_listView->model())->stringList();
        if (files.contains(file)) {
            return;
        }
        m_listView->model()->insertRow(m_listView->model()->rowCount());
        m_listView->model()->setData(m_listView->model()->index(m_listView->model()->rowCount()-1, 0), file);
    });
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        accept();
    });
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, [this]() {
        reject();
    });
}

FileDialog::~FileDialog()
{
    delete m_listView;
    delete m_buttonBox;
    delete m_fileDialog;
    delete ui;
}

QStringList FileDialog::selectedFiles() const
{
    return static_cast<QStringListModel*>(m_listView->model())->stringList();
}

void FileDialog::setFileMode(QFileDialog::FileMode mode) {
    m_fileDialog->setFileMode(mode);
}

void FileDialog::setAcceptMode(QFileDialog::AcceptMode mode) {
    m_fileDialog->setAcceptMode(mode);
}

void FileDialog::setDefaultSuffix(const QString &suffix) {
    m_fileDialog->setDefaultSuffix(suffix);
}

void FileDialog::setSidebarUrls(const QList<QUrl> &urls) {
    m_fileDialog->setSidebarUrls(urls);
}

void FileDialog::setOptions(QFileDialog::Options options) {
    m_fileDialog->setOptions(options);
}

void FileDialog::selectNameFilter(const QString &filter) {
    m_fileDialog->selectNameFilter(filter);
}

QString FileDialog::selectedNameFilter() const {
    return m_fileDialog->selectedNameFilter();
}
