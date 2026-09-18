// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "CustomPage.h"
#include "ui_CustomPage.h"

#include <QTabBar>

#include "Application.h"
#include "Filter.h"
#include "InstanceImportTask.h"
#include "Version.h"
#include "meta/Index.h"
#include "meta/VersionList.h"
#include "minecraft/VanillaInstanceCreationTask.h"
#include "ui/dialogs/NewInstanceDialog.h"

CustomPage::CustomPage(NewInstanceDialog* dialog, QWidget* parent) : QWidget(parent), dialog(dialog), ui(new Ui::CustomPage)
{
    ui->setupUi(this);
    connect(ui->versionList, &VersionSelectWidget::selectedVersionChanged, this, &CustomPage::setSelectedVersion);
    filterChanged();
    connect(ui->alphaFilter, &QCheckBox::stateChanged, this, &CustomPage::filterChanged);
    connect(ui->betaFilter, &QCheckBox::stateChanged, this, &CustomPage::filterChanged);
    connect(ui->snapshotFilter, &QCheckBox::stateChanged, this, &CustomPage::filterChanged);
    connect(ui->releaseFilter, &QCheckBox::stateChanged, this, &CustomPage::filterChanged);
    connect(ui->experimentsFilter, &QCheckBox::stateChanged, this, &CustomPage::filterChanged);
    connect(ui->refreshBtn, &QPushButton::clicked, this, &CustomPage::refresh);

    connect(ui->loaderVersionList, &VersionSelectWidget::selectedVersionChanged, this, &CustomPage::setSelectedLoaderVersion);
    connect(ui->noneFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->neoForgeFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->forgeFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->fabricFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->quiltFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->liteLoaderFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->legacyFabricFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->babricFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->btaFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->ornitheFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->riftFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->cleanroomFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->nilLoaderFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->cursedFabricFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->foxLoaderFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->fishModLoaderFilter, &QRadioButton::toggled, this, &CustomPage::loaderFilterChanged);
    connect(ui->loaderRefreshBtn, &QPushButton::clicked, this, &CustomPage::loaderRefresh);
}

void CustomPage::openedImpl()
{
    if (!initialized) {
        auto vlist = APPLICATION->metadataIndex()->get("net.minecraft");
        ui->versionList->initialize(vlist.get());
        initialized = true;
    } else {
        suggestCurrent();
    }
}

void CustomPage::refresh()
{
    ui->versionList->loadList(true);
}

void CustomPage::loaderRefresh()
{
    if (ui->noneFilter->isChecked() || !m_packagedLoaderUrl.isEmpty())
        return;
    ui->loaderVersionList->loadList(true);
}

void CustomPage::filterChanged()
{
    QStringList out;
    if (ui->alphaFilter->isChecked())
        out << "(alpha)";
    if (ui->betaFilter->isChecked())
        out << "(beta)";
    if (ui->snapshotFilter->isChecked())
        out << "(snapshot)";
    if (ui->releaseFilter->isChecked())
        out << "(release)";
    if (ui->experimentsFilter->isChecked())
        out << "(experiment)";
    auto regexp = out.join('|');
    ui->versionList->setFilter(BaseVersionList::TypeRole, Filters::regexp(QRegularExpression(regexp)));
}

void CustomPage::loaderFilterChanged()
{
    m_packagedLoaderUrl.clear();
    QString minecraftVersion;
    if (m_selectedVersion) {
        minecraftVersion = m_selectedVersion->descriptor();
    } else {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "AAA");  // empty list
        ui->loaderVersionList->setEmptyString(tr("No Minecraft version is selected."));
        ui->loaderVersionList->setEmptyMode(VersionListView::String);
        return;
    }
    if (ui->noneFilter->isChecked()) {
        m_selectedLoader.clear();
        m_selectedLoaderVersion.reset();
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "AAA");  // empty list
        ui->loaderVersionList->setEmptyString(tr("No mod loader is selected."));
        ui->loaderVersionList->setEmptyMode(VersionListView::String);
        suggestCurrent();
        return;
    } else if (ui->neoForgeFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "net.neoforged";
    } else if (ui->forgeFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "net.minecraftforge";
    } else if (ui->fabricFilter->isChecked()) {
        // FIXME: dirty hack because the launcher is unaware of Fabric's dependencies
        if (Version(minecraftVersion) >= Version("1.14"))  // Fabric/Quilt supported
            ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "");
        else                                                                                   // Fabric/Quilt unsupported
            ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "AAA");  // clear list
        m_selectedLoader = "net.fabricmc.fabric-loader";
    } else if (ui->quiltFilter->isChecked()) {
        // FIXME: dirty hack because the launcher is unaware of Quilt's dependencies (same as Fabric)
        if (Version(minecraftVersion) >= Version("1.14"))  // Fabric/Quilt supported
            ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "");
        else                                                                                   // Fabric/Quilt unsupported
            ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "AAA");  // clear list
        m_selectedLoader = "org.quiltmc.quilt-loader";
    } else if (ui->liteLoaderFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "com.mumfrey.liteloader";
    } else if (ui->legacyFabricFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "org.legacyfabric.fabric-loader";
    } else if (ui->babricFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "org.babric.fabric-loader";
    } else if (ui->btaFilter->isChecked()) {
        m_packagedLoaderUrl = QUrl("https://downloads.betterthanadventure.net/bta-client/release/v8.0.1/bta.v8.0.1.mmc.zip");
    } else if (ui->ornitheFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "net.ornithemc.loader";
    } else if (ui->riftFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "io.github.rift";
    } else if (ui->cleanroomFilter->isChecked()) {
        m_packagedLoaderUrl = QUrl("https://github.com/CleanroomMC/Cleanroom/releases/download/0.6.13-alpha/cleanroom-0.6.13-alpha.zip");
    } else if (ui->nilLoaderFilter->isChecked()) {
        // NilLoader is application-independent and intentionally has no
        // Minecraft dependency in its component metadata.
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "");
        m_selectedLoader = "com.unascribed.nilloader";
    } else if (ui->cursedFabricFilter->isChecked()) {
        m_packagedLoaderUrl =
            QUrl("https://github.com/BTW-Community/cursed-fabric-loader/releases/download/1.1.0-btw/Cursed-Fabric-BTW-MultiMC.zip");
    } else if (ui->foxLoaderFilter->isChecked()) {
        m_packagedLoaderUrl = QUrl("https://github.com/Fox2Code/FoxLoader/releases/download/1.3.3/FoxLoader-1.3.3-mmc.zip");
    } else if (ui->fishModLoaderFilter->isChecked()) {
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, minecraftVersion);
        m_selectedLoader = "net.xiaoyu233.fml";
    }

    if (!m_packagedLoaderUrl.isEmpty()) {
        m_selectedLoader.clear();
        m_selectedLoaderVersion.reset();
        ui->loaderVersionList->setExactFilter(BaseVersionList::ParentVersionRole, "AAA");
        ui->loaderVersionList->setEmptyString(tr("Atlas will download and import the official prebuilt instance."));
        ui->loaderVersionList->setEmptyMode(VersionListView::String);
        suggestCurrent();
        return;
    }

    auto vlist = APPLICATION->metadataIndex()->get(m_selectedLoader);
    ui->loaderVersionList->initialize(vlist.get());
    ui->loaderVersionList->selectRecommended();
    ui->loaderVersionList->setEmptyString(tr("No versions are currently available for Minecraft %1").arg(minecraftVersion));
}

CustomPage::~CustomPage()
{
    delete ui;
}

bool CustomPage::shouldDisplay() const
{
    return true;
}

void CustomPage::retranslate()
{
    ui->retranslateUi(this);
}

BaseVersion::Ptr CustomPage::selectedVersion() const
{
    return m_selectedVersion;
}

BaseVersion::Ptr CustomPage::selectedLoaderVersion() const
{
    return m_selectedLoaderVersion;
}

QString CustomPage::selectedLoader() const
{
    return m_selectedLoader;
}

QString CustomPage::selectedLoaderName() const
{
    if (ui->neoForgeFilter->isChecked()) {
        return ui->neoForgeFilter->text();
    }
    if (ui->forgeFilter->isChecked()) {
        return ui->forgeFilter->text();
    }
    if (ui->fabricFilter->isChecked()) {
        return ui->fabricFilter->text();
    }
    if (ui->quiltFilter->isChecked()) {
        return ui->quiltFilter->text();
    }
    if (ui->liteLoaderFilter->isChecked()) {
        return ui->liteLoaderFilter->text();
    }
    if (ui->legacyFabricFilter->isChecked()) return ui->legacyFabricFilter->text();
    if (ui->babricFilter->isChecked()) return ui->babricFilter->text();
    if (ui->btaFilter->isChecked()) return ui->btaFilter->text();
    if (ui->ornitheFilter->isChecked()) return ui->ornitheFilter->text();
    if (ui->riftFilter->isChecked()) return ui->riftFilter->text();
    if (ui->cleanroomFilter->isChecked()) return ui->cleanroomFilter->text();
    if (ui->nilLoaderFilter->isChecked()) return ui->nilLoaderFilter->text();
    if (ui->cursedFabricFilter->isChecked()) return ui->cursedFabricFilter->text();
    if (ui->foxLoaderFilter->isChecked()) return ui->foxLoaderFilter->text();
    if (ui->fishModLoaderFilter->isChecked()) return ui->fishModLoaderFilter->text();
    return QString();
}

void CustomPage::suggestCurrent()
{
    if (!isOpened) {
        return;
    }

    if (!m_selectedVersion) {
        dialog->setSuggestedPack();
        return;
    }

    if (!m_packagedLoaderUrl.isEmpty()) {
        const QString suggestedName = selectedLoaderName();
        dialog->setSuggestedPack(suggestedName, new InstanceImportTask(m_packagedLoaderUrl, true, this));
    }
    // There isn't a selected version if the version list is empty
    else if (ui->loaderVersionList->selectedVersion() == nullptr)
        dialog->setSuggestedPack(m_selectedVersion->descriptor(), new VanillaCreationTask(m_selectedVersion));
    else {
        QString suggestedName = QString("%1 %2").arg(m_selectedVersion->descriptor(), selectedLoaderName());
        dialog->setSuggestedPack(suggestedName, new VanillaCreationTask(m_selectedVersion, m_selectedLoader, m_selectedLoaderVersion));
    }
    dialog->setSuggestedIcon("default");
}

void CustomPage::setSelectedVersion(BaseVersion::Ptr version)
{
    m_selectedVersion = version;
    suggestCurrent();
    loaderFilterChanged();
}

void CustomPage::setSelectedLoaderVersion(BaseVersion::Ptr version)
{
    m_selectedLoaderVersion = version;
    suggestCurrent();
}
