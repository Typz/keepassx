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

#include "OptionDialog.h"
#include "ui_OptionDialog.h"
#include "HttpSettings.h"

OptionDialog::OptionDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OptionDialog)
{
    ui->setupUi(this);
    connect(ui->removeSharedEncryptionKeys, SIGNAL(clicked()), this, SIGNAL(removeSharedEncryptionKeys()));
    connect(ui->removeStoredPermissions, SIGNAL(clicked()), this, SIGNAL(removeStoredPermissions()));
}

OptionDialog::~OptionDialog()
{
    delete ui;
}

void OptionDialog::loadSettings()
{
    HttpSettings settings;
    ui->enableHttpServer->setChecked(settings.isEnabled());

    ui->showNotification->setChecked(settings.showNotification());
    ui->bestMatchOnly->setChecked(settings.bestMatchOnly());
    ui->unlockDatabase->setChecked(settings.unlockDatabase());
    ui->matchUrlScheme->setChecked(settings.matchUrlScheme());
    if (settings.sortByUsername())
        ui->sortByUsername->setChecked(true);
    else
        ui->sortByTitle->setChecked(true);

    ui->checkBoxLower->setChecked(settings.passwordUseLowercase());
    ui->checkBoxNumbers->setChecked(settings.passwordUseNumbers());
    ui->checkBoxUpper->setChecked(settings.passwordUseUppercase());
    ui->checkBoxSpecialChars->setChecked(settings.passwordUseSpecial());
    ui->checkBoxEnsureEvery->setChecked(settings.passwordEveryGroup());
    ui->checkBoxExcludeAlike->setChecked(settings.passwordExcludeAlike());
    ui->spinBoxLength->setValue(settings.passwordLength());

    ui->alwaysAllowAccess->setChecked(settings.alwaysAllowAccess());
    ui->alwaysAllowUpdate->setChecked(settings.alwaysAllowUpdate());
    ui->searchInAllDatabases->setChecked(settings.searchInAllDatabases());
    ui->httpPort->setValue(settings.listenerPort());
}

void OptionDialog::saveSettings()
{
    HttpSettings settings;
    settings.setEnabled(ui->enableHttpServer->isChecked());

    settings.setShowNotification(ui->showNotification->isChecked());
    settings.setBestMatchOnly(ui->bestMatchOnly->isChecked());
    settings.setUnlockDatabase(ui->unlockDatabase->isChecked());
    settings.setMatchUrlScheme(ui->matchUrlScheme->isChecked());
    settings.setSortByUsername(ui->sortByUsername->isChecked());

    settings.setPasswordUseLowercase(ui->checkBoxLower->isChecked());
    settings.setPasswordUseNumbers(ui->checkBoxNumbers->isChecked());
    settings.setPasswordUseUppercase(ui->checkBoxUpper->isChecked());
    settings.setPasswordUseSpecial(ui->checkBoxSpecialChars->isChecked());
    settings.setPasswordEveryGroup(ui->checkBoxEnsureEvery->isChecked());
    settings.setPasswordExcludeAlike(ui->checkBoxExcludeAlike->isChecked());
    settings.setPasswordLength(ui->spinBoxLength->value());

    settings.setAlwaysAllowAccess(ui->alwaysAllowAccess->isChecked());
    settings.setAlwaysAllowUpdate(ui->alwaysAllowUpdate->isChecked());
    settings.setSearchInAllDatabases(ui->searchInAllDatabases->isChecked());
    settings.setListenerPort(ui->httpPort->value());
}
