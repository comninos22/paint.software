#include "settingsdialog.h"
#include "i18n.h"
#include "theme.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QListWidget>
#include <QStackedWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle(TR("Paramètres"));
    resize(560, 400);

    auto *root = new QVBoxLayout(this);
    auto *body = new QHBoxLayout;

    // ---- Category list (left) ----
    m_categories = new QListWidget;
    m_categories->setMinimumWidth(180);
    m_categories->setMaximumWidth(230);
    m_categories->addItem(TR("Interface utilisateur"));
    m_categories->addItem(TR("Canevas"));
    m_categories->addItem(TR("Outils"));
    m_categories->addItem(TR("Aide"));
    body->addWidget(m_categories);

    // ---- Pages (right) ----
    m_pages = new QStackedWidget;
    m_pages->addWidget(buildUserInterfacePage());
    m_pages->addWidget(buildCanvasPage());
    m_pages->addWidget(buildPlaceholderPage(
        "Largeur du pinceau ( [ et ] pour ajuster )"));
    m_pages->addWidget(buildPlaceholderPage(
        "Astuce : F5–F8 affichent/masquent les fenêtres Outils, Historique, Calques et Couleurs."));
    body->addWidget(m_pages, 1);

    connect(m_categories, &QListWidget::currentRowChanged,
            m_pages, &QStackedWidget::setCurrentIndex);
    m_categories->setCurrentRow(0);

    root->addLayout(body, 1);

    // ---- Close button ----
    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    m_closeBtn = new QPushButton(TR("Fermer"));
    auto *closeBtn = m_closeBtn;
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnRow->addWidget(closeBtn);
    root->addLayout(btnRow);
}

QWidget *SettingsDialog::buildUserInterfacePage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    auto *form = new QFormLayout;

    // Color Scheme: Default / Light / Dark, applied immediately.
    m_schemeCombo = new QComboBox;
    m_schemeCombo->addItem(TR("Défaut (système)"), int(Theme::Scheme::Default));
    m_schemeCombo->addItem(TR("Clair"),            int(Theme::Scheme::Light));
    m_schemeCombo->addItem(TR("Sombre"),           int(Theme::Scheme::Dark));
    m_schemeCombo->setCurrentIndex(int(Theme::scheme()));
    connect(m_schemeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
        Theme::setScheme(static_cast<Theme::Scheme>(m_schemeCombo->itemData(i).toInt()));
        Theme::saveToSettings();
        emit colorSchemeChanged();
    });
    m_schemeLabel = new QLabel(TR("Thème :"));
    form->addRow(m_schemeLabel, m_schemeCombo);

    // Language choices use native names so users can always find their language.
    m_languageCombo = new QComboBox;
    m_languageCombo->setObjectName("languageCombo");
    m_languageCombo->addItem(QStringLiteral("Français"), int(I18n::Lang::French));
    m_languageCombo->addItem(QStringLiteral("English"), int(I18n::Lang::English));
    m_languageCombo->addItem(QStringLiteral("Ελληνικά"), int(I18n::Lang::Greek));
    m_languageCombo->setCurrentIndex(m_languageCombo->findData(int(I18n::language())));
    connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
        I18n::setLanguage(static_cast<I18n::Lang>(m_languageCombo->itemData(i).toInt()));
        I18n::saveToSettings();
        I18n::applyQtTranslations();
        retranslate();
        emit languageChanged();
    });
    m_languageLabel = new QLabel(TR("Langue :"));
    form->addRow(m_languageLabel, m_languageCombo);

    layout->addLayout(form);

    m_note = new QLabel(TR("Le changement de langue est appliqué immédiatement."));
    auto *note = m_note;
    note->setStyleSheet("color: #4a90d9;");
    note->setWordWrap(true);
    layout->addWidget(note);

    layout->addStretch();
    return page;
}

QWidget *SettingsDialog::buildCanvasPage() {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);

    QSettings s("PaintDali", "PaintDali");

    m_checkerLabel = new QLabel(TR("Luminosité du damier de transparence :"));
    auto *label = m_checkerLabel;
    label->setWordWrap(true);
    layout->addWidget(label);

    m_checkerSlider = new QSlider(Qt::Horizontal);
    m_checkerSlider->setRange(0, 100);
    m_checkerSlider->setValue(s.value("canvas/checkerBrightness", 80).toInt());
    connect(m_checkerSlider, &QSlider::valueChanged, this, [this](int v) {
        QSettings s2("PaintDali", "PaintDali");
        s2.setValue("canvas/checkerBrightness", v);
        emit canvasSettingsChanged();
    });
    layout->addWidget(m_checkerSlider);

    layout->addStretch();
    return page;
}

QWidget *SettingsDialog::buildPlaceholderPage(const QString &text) {
    auto *page = new QWidget;
    auto *layout = new QVBoxLayout(page);
    auto *label = new QLabel(TR(text));
    label->setProperty("translationKey", text);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignTop);
    layout->addWidget(label);
    layout->addStretch();
    return page;
}

void SettingsDialog::retranslate() {
    setWindowTitle(TR("Paramètres"));
    const QStringList categories = {TR("Interface utilisateur"), TR("Canevas"), TR("Outils"), TR("Aide")};
    for (int i = 0; i < categories.size(); ++i) m_categories->item(i)->setText(categories[i]);
    m_closeBtn->setText(TR("Fermer"));
    m_languageLabel->setText(TR("Langue :"));
    m_schemeLabel->setText(TR("Thème :"));
    m_note->setText(TR("Le changement de langue est appliqué immédiatement."));
    m_checkerLabel->setText(TR("Luminosité du damier de transparence :"));
    const QSignalBlocker blocker(m_schemeCombo);
    m_schemeCombo->setItemText(0, TR("Défaut (système)"));
    m_schemeCombo->setItemText(1, TR("Clair"));
    m_schemeCombo->setItemText(2, TR("Sombre"));
    for (QLabel *label : m_pages->findChildren<QLabel *>()) {
        const QString key = label->property("translationKey").toString();
        if (!key.isEmpty()) label->setText(TR(key));
    }
}
