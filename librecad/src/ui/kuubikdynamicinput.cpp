#include "kuubikdynamicinput.h"

#include <cmath>

#include <QKeyEvent>
#include <QLabel>
#include <QSettings>
#include <QStringList>
#include <QWidget>

#include "rs_math.h"

KuubikDynamicInput::KuubikDynamicInput(QWidget* canvas)
{
    label = new QLabel(canvas);
    label->setObjectName(QStringLiteral("kuubikDynamicInput"));
    label->setAttribute(Qt::WA_TransparentForMouseEvents);
    label->setTextFormat(Qt::RichText);
    label->setStyleSheet(QStringLiteral(
        "QLabel#kuubikDynamicInput { background:#202a38; color:#e8edf5;"
        " border:1px solid #5f738c; border-radius:2px; padding:3px 5px; }"));
    label->hide();
}

KuubikDynamicInput::~KuubikDynamicInput()
{
    delete label;
}

void KuubikDynamicInput::update(const RS_Vector& start,
                                const RS_Vector& cursor,
                                int cursorX, int cursorY)
{
    if (!QSettings().value(QStringLiteral("KuubikStatus/DynamicInput"), true).toBool()) {
        hide();
        return;
    }
    startPoint = start;
    cursorPoint = cursor;
    if (!start.valid || !cursor.valid) {
        hide();
        return;
    }
    refresh();
    label->adjustSize();
    QWidget* canvas = label->parentWidget();
    int x = cursorX + 18;
    int y = cursorY + 18;
    if (canvas != nullptr) {
        x = qMin(x, canvas->width() - label->width() - 6);
        y = qMin(y, canvas->height() - label->height() - 6);
    }
    label->move(qMax(6, x), qMax(6, y));
    label->show();
    label->raise();
}

bool KuubikDynamicInput::handleKey(QKeyEvent* event)
{
    if (!QSettings().value(QStringLiteral("KuubikStatus/DynamicInput"), true).toBool()) return false;
    if (event == nullptr || !startPoint.valid) return false;
    const bool showDistance = QSettings().value(
        QStringLiteral("KuubikStatus/DynamicShowDistance"), true).toBool();
    const bool showAngle = QSettings().value(
        QStringLiteral("KuubikStatus/DynamicShowAngle"), true).toBool();
    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
        if (showDistance && showAngle) {
            editingAngle = !editingAngle;
        } else {
            editingAngle = showAngle;
        }
        refresh();
        event->accept();
        return true;
    }
    if (!showDistance && showAngle) editingAngle = true;
    if (showDistance && !showAngle) editingAngle = false;
    QString* target = editingAngle ? &angleText : &lengthText;
    if (event->key() == Qt::Key_Backspace) {
        target->chop(1);
        refresh();
        event->accept();
        return true;
    }
    const QString text = event->text();
    if (text.size() == 1
        && (text.at(0).isDigit() || text == QStringLiteral(".")
            || text == QStringLiteral(",") || text == QStringLiteral("-"))) {
        target->append(text);
        refresh();
        event->accept();
        return true;
    }
    return false;
}

bool KuubikDynamicInput::hasTypedValue() const
{
    return !lengthText.isEmpty() || !angleText.isEmpty();
}

RS_Vector KuubikDynamicInput::endpoint() const
{
    if (!startPoint.valid || !cursorPoint.valid) return RS_Vector(false);
    const RS_Vector delta = cursorPoint - startPoint;
    double length = delta.magnitude();
    double angle = delta.angle();
    double value = 0.0;
    const bool showDistance = QSettings().value(
        QStringLiteral("KuubikStatus/DynamicShowDistance"), true).toBool();
    const bool showAngle = QSettings().value(
        QStringLiteral("KuubikStatus/DynamicShowAngle"), true).toBool();
    if (showDistance && parse(lengthText, value)) length = value;
    if (showAngle && parse(angleText, value)) angle = RS_Math::deg2rad(value);
    return startPoint + RS_Vector::polar(length, angle);
}

void KuubikDynamicInput::resetValues()
{
    lengthText.clear();
    angleText.clear();
    editingAngle = false;
}

void KuubikDynamicInput::hide()
{
    if (label != nullptr) label->hide();
}

void KuubikDynamicInput::refresh()
{
    if (label == nullptr || !startPoint.valid || !cursorPoint.valid) return;
    const RS_Vector delta = cursorPoint - startPoint;
    const QString length = lengthText.isEmpty()
                               ? QString::number(delta.magnitude(), 'f', 2)
                               : lengthText;
    const QString angle = angleText.isEmpty()
                              ? QString::number(RS_Math::rad2deg(delta.angle()),
                                                'f', 2)
                              : angleText;
    const QString lengthColor = editingAngle ? QStringLiteral("#e8edf5")
                                              : QStringLiteral("#63b9ff");
    const QString angleColor = editingAngle ? QStringLiteral("#63b9ff")
                                             : QStringLiteral("#e8edf5");
    const bool showDistance = QSettings().value(
        QStringLiteral("KuubikStatus/DynamicShowDistance"), true).toBool();
    const bool showAngle = QSettings().value(
        QStringLiteral("KuubikStatus/DynamicShowAngle"), true).toBool();
    QStringList fields;
    if (showDistance) {
        fields.append(QStringLiteral("<span style='color:%1'>L %2</span>")
                          .arg(lengthColor, length));
    }
    if (showAngle) {
        fields.append(QStringLiteral("<span style='color:%1'>A %2&deg;</span>")
                          .arg(angleColor, angle));
    }
    label->setText(fields.join(QStringLiteral("&nbsp;&nbsp;")));
}

bool KuubikDynamicInput::parse(const QString& text, double& value)
{
    if (text.isEmpty()) return false;
    QString normalized = text;
    normalized.replace(',', '.');
    bool ok = false;
    const double parsed = normalized.toDouble(&ok);
    if (ok) value = parsed;
    return ok;
}
