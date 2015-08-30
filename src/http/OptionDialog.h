/*
 *  Copyright (C) 2013 Francois Ferrand <thetypz@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QtGui/QWidget>

namespace Ui {
class OptionDialog;
}

class OptionDialog : public QWidget
{
    Q_OBJECT

public:
    explicit OptionDialog(QWidget *parent = 0);
    ~OptionDialog();

public Q_SLOTS:
    void loadSettings();
    void saveSettings();

Q_SIGNALS:
    void removeSharedEncryptionKeys();
    void removeStoredPermissions();

private:
    Ui::OptionDialog *ui;
};

#endif // OPTIONDIALOG_H
