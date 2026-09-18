// SPDX-License-Identifier: GPL-3.0-only

#include "BedrockPage.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QFont>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>

BedrockPage::BedrockPage(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    auto* title = new QLabel(tr("Minecraft: Bedrock Edition"), this);
    QFont titleFont = title->font();
    titleFont.setPointSize(titleFont.pointSize() + 4);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* description = new QLabel(
        tr("Experimental Windows support powered by the open-source BedrockLauncher backend. "
           "It can manage multiple Bedrock releases and previews. A legitimate Minecraft for Windows "
           "entitlement or Game Pass subscription is required."),
        this);
    description->setWordWrap(true);

    auto* warning = new QLabel(
        tr("This experimental build keeps Bedrock isolated from stable Atlas releases. "
           "Worlds should be backed up before changing versions."),
        this);
    warning->setWordWrap(true);

    auto* launchButton = new QPushButton(tr("Manage Bedrock versions"), this);
    connect(launchButton, &QPushButton::clicked, this, &BedrockPage::launchBackend);

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addWidget(warning);
    layout->addSpacing(12);
    layout->addWidget(launchButton);
    layout->addStretch();
}

bool BedrockPage::shouldDisplay() const
{
#if defined(Q_OS_WIN) && defined(ATLAS_ENABLE_BEDROCK_EXPERIMENTAL)
    return true;
#else
    return false;
#endif
}

void BedrockPage::launchBackend()
{
    const QString backend = QCoreApplication::applicationDirPath() + "/bedrock/BedrockLauncher.exe";
    if (!QFileInfo::exists(backend)) {
        QMessageBox::critical(this, tr("Bedrock backend missing"),
                              tr("The Bedrock backend was not found. Install the complete Atlas Bedrock Experimental artifact."));
        return;
    }

    if (!QProcess::startDetached(backend, {})) {
        QMessageBox::critical(this, tr("Could not start Bedrock"), tr("Atlas could not start the Bedrock version manager."));
    }
}
