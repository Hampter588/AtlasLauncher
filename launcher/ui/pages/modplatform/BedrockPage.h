// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <QWidget>

#include "ui/pages/BasePage.h"

class BedrockPage final : public QWidget, public BasePage {
    Q_OBJECT

   public:
    explicit BedrockPage(QWidget* parent = nullptr);

    QString displayName() const override { return tr("Bedrock (Experimental)"); }
    QIcon icon() const override { return QIcon::fromTheme("applications-games"); }
    QString id() const override { return "bedrock-experimental"; }
    QString helpPage() const override { return {}; }
    bool shouldDisplay() const override;
    void retranslate() override {}

   private slots:
    void launchBackend();
};
