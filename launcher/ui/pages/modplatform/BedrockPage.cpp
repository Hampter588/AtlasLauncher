// SPDX-License-Identifier: GPL-3.0-only

#include "BedrockPage.h"

#include <QFont>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include <QTableWidget>
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
        tr("Native experimental support for Minecraft for Windows. Atlas detects installed Stable and Preview "
           "editions and launches them directly. A legitimate Minecraft entitlement or Game Pass subscription is required."),
        this);
    description->setWordWrap(true);

    auto* warning = new QLabel(
        tr("This experimental build keeps Bedrock isolated from stable Atlas releases. "
           "Back up your worlds before testing experimental launcher features."),
        this);
    warning->setWordWrap(true);

    m_status = new QLabel(tr("Checking installed Bedrock editions…"), this);
    m_installations = new QTableWidget(this);
    m_installations->setColumnCount(3);
    m_installations->setHorizontalHeaderLabels({ tr("Edition"), tr("Version"), tr("Package") });
    m_installations->horizontalHeader()->setStretchLastSection(true);
    m_installations->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_installations->setSelectionMode(QAbstractItemView::SingleSelection);
    m_installations->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto* refreshButton = new QPushButton(tr("Refresh"), this);
    m_launchButton = new QPushButton(tr("Launch selected edition"), this);
    m_launchButton->setEnabled(false);
    connect(refreshButton, &QPushButton::clicked, this, &BedrockPage::refreshInstallations);
    connect(m_launchButton, &QPushButton::clicked, this, &BedrockPage::launchSelected);
    connect(m_installations, &QTableWidget::itemSelectionChanged, this,
            [this] { m_launchButton->setEnabled(!m_installations->selectedItems().isEmpty()); });

    m_detector = new QProcess(this);
    connect(m_detector, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this,
            [this](int exitCode, QProcess::ExitStatus) { processDetectionResult(exitCode); });

    layout->addWidget(title);
    layout->addWidget(description);
    layout->addWidget(warning);
    layout->addSpacing(12);
    layout->addWidget(m_status);
    layout->addWidget(m_installations);
    layout->addWidget(refreshButton);
    layout->addWidget(m_launchButton);
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

void BedrockPage::openedImpl()
{
    refreshInstallations();
}

void BedrockPage::refreshInstallations()
{
#if defined(Q_OS_WIN) && defined(ATLAS_ENABLE_BEDROCK_EXPERIMENTAL)
    if (m_detector->state() != QProcess::NotRunning) {
        return;
    }

    m_status->setText(tr("Checking installed Bedrock editions…"));
    m_installations->setRowCount(0);
    m_launchButton->setEnabled(false);
    const QString script =
        "Get-AppxPackage | Where-Object { $_.Name -in "
        "@('Microsoft.MinecraftUWP','Microsoft.MinecraftWindowsBeta') } | "
        "Select-Object Name,@{Name='Version';Expression={$_.Version.ToString()}},PackageFamilyName | "
        "ConvertTo-Json -Compress";
    m_detector->start("powershell.exe", { "-NoProfile", "-NonInteractive", "-Command", script });
#endif
}

void BedrockPage::processDetectionResult(int exitCode)
{
    if (exitCode != 0) {
        m_status->setText(tr("Bedrock package detection failed."));
        return;
    }

    const QByteArray output = m_detector->readAllStandardOutput().trimmed();
    if (output.isEmpty()) {
        m_status->setText(tr("Minecraft for Windows or Preview is not installed for this Windows account."));
        return;
    }

    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(output, &error);
    if (error.error != QJsonParseError::NoError) {
        m_status->setText(tr("Windows returned package information Atlas could not read."));
        return;
    }

    QJsonArray packages;
    if (document.isArray()) {
        packages = document.array();
    } else if (document.isObject()) {
        packages.append(document.object());
    }

    for (const auto& value : packages) {
        const auto package = value.toObject();
        const QString name = package.value("Name").toString();
        const QString family = package.value("PackageFamilyName").toString();
        const int row = m_installations->rowCount();
        m_installations->insertRow(row);
        m_installations->setItem(row, 0,
                                 new QTableWidgetItem(name == "Microsoft.MinecraftWindowsBeta" ? tr("Bedrock Preview")
                                                                                               : tr("Bedrock Stable")));
        m_installations->setItem(row, 1, new QTableWidgetItem(package.value("Version").toString()));
        auto* familyItem = new QTableWidgetItem(family);
        familyItem->setData(Qt::UserRole, family);
        m_installations->setItem(row, 2, familyItem);
    }

    m_status->setText(tr("Found %1 installed Bedrock edition(s).").arg(packages.size()));
    if (!packages.isEmpty()) {
        m_installations->selectRow(0);
    }
}

void BedrockPage::launchSelected()
{
    const int row = m_installations->currentRow();
    if (row < 0) {
        return;
    }

    const QString family = m_installations->item(row, 2)->data(Qt::UserRole).toString();
    const QString appId = QString("shell:AppsFolder\\%1!App").arg(family);

    if (!QProcess::startDetached("explorer.exe", { appId })) {
        QMessageBox::critical(this, tr("Could not start Bedrock"), tr("Atlas could not launch the selected Bedrock edition."));
    }
}
