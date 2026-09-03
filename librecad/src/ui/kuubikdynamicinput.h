#ifndef KUUBIKDYNAMICINPUT_H
#define KUUBIKDYNAMICINPUT_H

#include <QString>

#include "rs_vector.h"

class QLabel;
class QKeyEvent;
class QWidget;

class KuubikDynamicInput {
public:
    explicit KuubikDynamicInput(QWidget* canvas);
    ~KuubikDynamicInput();

    void update(const RS_Vector& start, const RS_Vector& cursor,
                int cursorX, int cursorY);
    bool handleKey(QKeyEvent* event);
    bool hasTypedValue() const;
    RS_Vector endpoint() const;
    void resetValues();
    void hide();

private:
    void refresh();
    static bool parse(const QString& text, double& value);

    QLabel* label = nullptr;
    RS_Vector startPoint {false};
    RS_Vector cursorPoint {false};
    QString lengthText;
    QString angleText;
    bool editingAngle = false;
};

#endif
