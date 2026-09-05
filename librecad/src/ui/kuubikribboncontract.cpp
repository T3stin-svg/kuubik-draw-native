/****************************************************************************
** Kuubik Draw SARibbon interaction evidence. GPLv2; see LICENSE.
****************************************************************************/

#include "kuubikribbon.h"
#include "SARibbon.h"

#include <QApplication>
#include <QImage>
#include <QJsonArray>
#include <QKeyEvent>
#include <QMouseEvent>

QJsonObject KuubikRibbon::interactionContract()
{
    QJsonObject result;
    const int previousTab = tabs->currentIndex();
    QPointer<QWidget> previousFocus = QApplication::focusWidget();
    auto* tabBar = tabs->ribbonTabBar();
    auto clickTab = [this, tabBar](int index) {
        const QRect rectangle = tabBar->tabRect(index);
        if (!tabBar->isVisibleTo(this) || !rectangle.isValid()
            || !tabBar->rect().contains(rectangle)) return false;
        const QPointF point(rectangle.center());
        QMouseEvent press(QEvent::MouseButtonPress, point, Qt::LeftButton,
                          Qt::LeftButton, Qt::NoModifier);
        QMouseEvent release(QEvent::MouseButtonRelease, point, Qt::LeftButton,
                            Qt::NoButton, Qt::NoModifier);
        QApplication::sendEvent(tabBar, &press);
        QApplication::sendEvent(tabBar, &release);
        QApplication::processEvents();
        updateCollapsedPanels();
        QApplication::processEvents();
        return tabs->currentIndex() == index;
    };

    bool tabsPassed = true;
    QJsonArray tabResults;
    for (int index = 0; index < tabBar->count(); ++index) {
        const bool clicked = clickTab(index);
        QJsonObject tab;
        tab.insert("name", tabBar->tabText(index));
        tab.insert("activatedByMouse", clicked);
        tabResults.append(tab);
        tabsPassed = tabsPassed && clicked;
    }
    result.insert("tabs", tabResults);
    result.insert("tabsPassed", tabsPassed);

    // GRID is a reversible native view toggle, not a document-modifying mock action.
    const bool viewActivated = clickTab(3);
    QToolButton* gridButton = buttonForAction(QStringLiteral("ViewGrid"));
    QAction* grid = actions.value(QStringLiteral("ViewGrid"), nullptr);
    const bool gridAvailable = viewActivated && gridButton != nullptr && grid != nullptr
        && gridButton->isVisibleTo(this) && gridButton->isEnabled() && grid->isCheckable();
    bool keyboardToggled = false;
    bool keyboardRestored = false;
    bool focusReached = false;
    int gridTriggerCount = 0;
    if (gridAvailable) {
        const bool originalChecked = grid->isChecked();
        const auto connection = connect(grid, &QAction::triggered, this,
                                        [&gridTriggerCount](bool) { ++gridTriggerCount; });
        gridButton->setFocus(Qt::TabFocusReason);
        QApplication::processEvents();
        focusReached = gridButton->hasFocus();
        auto sendSpace = [gridButton] {
            QKeyEvent press(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, QStringLiteral(" "));
            QKeyEvent release(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, QStringLiteral(" "));
            QApplication::sendEvent(gridButton, &press);
            QApplication::sendEvent(gridButton, &release);
            QApplication::processEvents();
        };
        sendSpace();
        keyboardToggled = gridTriggerCount == 1 && grid->isChecked() != originalChecked
            && gridButton->isChecked() == grid->isChecked();
        sendSpace();
        keyboardRestored = gridTriggerCount == 2 && grid->isChecked() == originalChecked
            && gridButton->isChecked() == originalChecked;
        // Restore even if a keyboard assertion failed, without hiding the failure.
        if (grid->isChecked() != originalChecked) grid->trigger();
        disconnect(connection);
    }
    result.insert("gridAvailable", gridAvailable);
    result.insert("keyboardFocusReached", focusReached);
    result.insert("spaceToggledNativeGrid", keyboardToggled);
    result.insert("spaceRestoredNativeGrid", keyboardRestored);
    result.insert("gridTriggerCount", gridTriggerCount);

    const bool homeActivated = clickTab(0);
    QAction* line = actions.value(QStringLiteral("DrawLine"), nullptr);
    QToolButton* lineButton = buttonForAction(QStringLiteral("DrawLine"));
    bool presentationStable = false;
    bool enabledMirrored = false;
    bool visibilityPreserved = false;
    if (homeActivated && line != nullptr && lineButton != nullptr) {
        const QString nativeText = line->text();
        const QIcon nativeIcon = line->icon();
        const QString kuubikText = lineButton->text();
        const QImage kuubikIcon = lineButton->icon().pixmap(24, 24).toImage();
        const bool nativeEnabled = line->isEnabled();
        const bool nativeVisible = line->isVisible();
        line->setText(QStringLiteral("Kuubik test construction variant"));
        line->setIcon(QIcon());
        QApplication::processEvents();
        presentationStable = lineButton->text() == kuubikText
            && !kuubikIcon.isNull() && lineButton->icon().pixmap(24, 24).toImage() == kuubikIcon;
        line->setEnabled(false);
        QApplication::processEvents();
        enabledMirrored = !lineButton->isEnabled();
        visibilityPreserved = line->isVisible() == nativeVisible;
        line->setText(nativeText);
        line->setIcon(nativeIcon);
        line->setEnabled(nativeEnabled);
        QApplication::processEvents();
        enabledMirrored = enabledMirrored && lineButton->isEnabled() == nativeEnabled;
        presentationStable = presentationStable && lineButton->text() == kuubikText
            && lineButton->icon().pixmap(24, 24).toImage() == kuubikIcon;
    }
    result.insert("nativePresentationChangesPreserveKuubik", presentationStable);
    result.insert("nativeEnabledMirroredAndRestored", enabledMirrored);
    result.insert("nativeVisibilityPreserved", visibilityPreserved);
    const bool tabRestored = clickTab(previousTab);
    if (previousFocus != nullptr) previousFocus->setFocus(Qt::OtherFocusReason);
    QApplication::processEvents();
    result.insert("tabRestored", tabRestored);
    const bool passed = tabsPassed && gridAvailable && focusReached && keyboardToggled
        && keyboardRestored && presentationStable && enabledMirrored && visibilityPreserved && tabRestored;
    result.insert("status", passed ? "PASS" : "FAIL");
    return result;
}
