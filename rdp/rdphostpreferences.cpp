/*
    SPDX-FileCopyrightText: 2007-2012 Urs Wolfer <uwolfer@kde.org>
    SPDX-FileCopyrightText: 2012 AceLan Kao <acelan@acelan.idv.tw>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rdphostpreferences.h"

#include "settings.h"

#include <QScreen>
#include <QWindow>
#include <QGuiApplication>

static const QStringList keymaps = (QStringList()
    << QStringLiteral("ar")
    << QStringLiteral("cs")
    << QStringLiteral("da")
    << QStringLiteral("de")
    << QStringLiteral("de-ch")
    << QStringLiteral("en-dv")
    << QStringLiteral("en-gb")
    << QStringLiteral("en-us")
    << QStringLiteral("es")
    << QStringLiteral("et")
    << QStringLiteral("fi")
    << QStringLiteral("fo")
    << QStringLiteral("fr")
    << QStringLiteral("fr-be")
    << QStringLiteral("fr-ca")
    << QStringLiteral("fr-ch")
    << QStringLiteral("he")
    << QStringLiteral("hr")
    << QStringLiteral("hu")
    << QStringLiteral("is")
    << QStringLiteral("it")
    << QStringLiteral("ja")
    << QStringLiteral("ko")
    << QStringLiteral("lt")
    << QStringLiteral("lv")
    << QStringLiteral("mk")
    << QStringLiteral("nl")
    << QStringLiteral("nl-be")
    << QStringLiteral("no")
    << QStringLiteral("pl")
    << QStringLiteral("pt")
    << QStringLiteral("pt-br")
    << QStringLiteral("ru")
    << QStringLiteral("sl")
    << QStringLiteral("sv")
    << QStringLiteral("th")
    << QStringLiteral("tr")
);

static const int defaultKeymap = 7; // en-us

inline int keymap2int(const QString &keymap)
{
    const int index = keymaps.lastIndexOf(keymap);
    return (index == -1) ? defaultKeymap : index;
}

inline QString int2keymap(int layout)
{
    if (layout >= 0 && layout < keymaps.count())
        return keymaps.at(layout);
    else
        return keymaps.at(defaultKeymap);
}

RdpHostPreferences::RdpHostPreferences(KConfigGroup configGroup, QObject *parent)
  : HostPreferences(configGroup, parent)
{
}

RdpHostPreferences::~RdpHostPreferences()
{
}

QWidget* RdpHostPreferences::createProtocolSpecificConfigPage()
{
    QWidget *rdpPage = new QWidget();
    rdpUi.setupUi(rdpPage);

    rdpUi.kcfg_Height->setValue(height());
    rdpUi.kcfg_Width->setValue(width());
    rdpUi.kcfg_Resolution->setCurrentIndex(int(resolution()));
    rdpUi.kcfg_Acceleration->setCurrentIndex(int(acceleration()));
    rdpUi.kcfg_ColorDepth->setCurrentIndex(int(colorDepth()));
    rdpUi.kcfg_KeyboardLayout->setCurrentIndex(keymap2int(keyboardLayout()));

    // Have to call updateWidthHeight() here
    // We leverage the final part of this function to enable/disable kcfg_Height and kcfg_Width
    updateWidthHeight(resolution());

    connect(rdpUi.kcfg_Resolution, &QComboBox::currentIndexChanged, this, [this](int index) {
        updateWidthHeight(Resolution(index));
    });

    // Color depth depends on acceleration method, with the better ones only working with 32-bit
    // color. So ensure we reflect that in the settings UI.
    updateColorDepth(acceleration());
    connect(rdpUi.kcfg_Acceleration, &QComboBox::currentIndexChanged, this, [this](int index) {
        updateColorDepth(Acceleration(index));
    });

    return rdpPage;
}

void RdpHostPreferences::updateWidthHeight(Resolution resolution)
{
    switch (resolution) {
    case Resolution::Small:
        rdpUi.kcfg_Width->setValue(1280);
        rdpUi.kcfg_Height->setValue(720);
        break;
    case Resolution::Medium:
        rdpUi.kcfg_Height->setValue(1600);
        rdpUi.kcfg_Width->setValue(900);
        break;
    case Resolution::Large:
        rdpUi.kcfg_Height->setValue(1920);
        rdpUi.kcfg_Width->setValue(1080);
        break;
    case Resolution::MatchWindow:
        rdpUi.kcfg_Height->setValue(-1);
        rdpUi.kcfg_Width->setValue(-1);
        break;
    case Resolution::MatchScreen: {
        QWindow *window = rdpUi.kcfg_Width->window()->windowHandle();
        QScreen *screen = window ? window->screen() : qGuiApp->primaryScreen();
        const QSize size = screen->size() * screen->devicePixelRatio();

        rdpUi.kcfg_Width->setValue(size.width());
        rdpUi.kcfg_Height->setValue(size.height());
        break;
    }
    case Resolution::Custom:
    default:
        break;
    }

    const bool enabled = resolution == Resolution::Custom;

    rdpUi.kcfg_Height->setEnabled(enabled);
    rdpUi.kcfg_Width->setEnabled(enabled);
    rdpUi.heightLabel->setEnabled(enabled);
    rdpUi.widthLabel->setEnabled(enabled);
}

void RdpHostPreferences::acceptConfig()
{
    HostPreferences::acceptConfig();

    setHeight(rdpUi.kcfg_Height->value());
    setWidth(rdpUi.kcfg_Width->value());
    setResolution(Resolution(rdpUi.kcfg_Resolution->currentIndex()));
    setAcceleration(Acceleration(rdpUi.kcfg_Acceleration->currentIndex()));
    setColorDepth(ColorDepth(rdpUi.kcfg_ColorDepth->currentIndex()));
    setKeyboardLayout(int2keymap(rdpUi.kcfg_KeyboardLayout->currentIndex()));
    setSound(Sound(rdpUi.kcfg_Sound->currentIndex()));
    setShareMedia(rdpUi.kcfg_shareMedia->text());
}

RdpHostPreferences::Resolution RdpHostPreferences::resolution() const
{
    return Resolution(m_configGroup.readEntry("resolution", Settings::resolution()));
}

void RdpHostPreferences::setResolution(Resolution resolution)
{
    m_configGroup.writeEntry("resolution", int(resolution));
}

RdpHostPreferences::Acceleration RdpHostPreferences::acceleration() const
{
    return Acceleration(m_configGroup.readEntry("acceleration", Settings::acceleration()));
}

void RdpHostPreferences::setAcceleration(Acceleration acceleration)
{
    m_configGroup.writeEntry("acceleration", int(acceleration));
}

void RdpHostPreferences::setColorDepth(ColorDepth colorDepth)
{
    m_configGroup.writeEntry("colorDepth", int(colorDepth));
}

RdpHostPreferences::ColorDepth RdpHostPreferences::colorDepth() const
{
    return ColorDepth(m_configGroup.readEntry("colorDepth", Settings::colorDepth()));
}

void RdpHostPreferences::setKeyboardLayout(const QString &keyboardLayout)
{
    if (!keyboardLayout.isNull())
        m_configGroup.writeEntry("keyboardLayout", keymap2int(keyboardLayout));
}

QString RdpHostPreferences::keyboardLayout() const
{
    return int2keymap(m_configGroup.readEntry("keyboardLayout", Settings::keyboardLayout()));
}

void RdpHostPreferences::setSound(Sound sound)
{
    m_configGroup.writeEntry("sound", int(sound));
}

RdpHostPreferences::Sound RdpHostPreferences::sound() const
{
    return Sound(m_configGroup.readEntry("sound", Settings::sound()));
}

void RdpHostPreferences::setShareMedia(const QString &shareMedia)
{
    if (!shareMedia.isNull())
        m_configGroup.writeEntry("shareMedia", shareMedia);
}

QString RdpHostPreferences::shareMedia() const
{
    return m_configGroup.readEntry("shareMedia", Settings::shareMedia());
}

void RdpHostPreferences::updateColorDepth(Acceleration acceleration)
{
    switch (acceleration) {
    case Acceleration::ForceGraphicsPipeline:
    case Acceleration::ForceRemoteFx:
        rdpUi.kcfg_ColorDepth->setEnabled(false);
        rdpUi.kcfg_ColorDepth->setCurrentIndex(0);
        break;
    case Acceleration::Disabled:
    case Acceleration::Auto:
        rdpUi.kcfg_ColorDepth->setEnabled(true);
    }
}


