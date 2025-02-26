#pragma once
#include <stdlib.h>

typedef int8_t POS_T; // ќпредел€ем POS_T как 8-битный целочисленный тип дл€ хранени€ координат

// —труктура move_pos описывает ход в игре
struct move_pos
{
    POS_T x, y;              //  оординаты начальной позиции фигуры (откуда)
    POS_T x2, y2;            //  оординаты конечной позиции фигуры (куда)
    POS_T xb = -1, yb = -1; //  оординаты побитой фигуры (если ход с рубкой, иначе -1)

    //  онструктор дл€ простого хода (без рубки)
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2) : x(x), y(y), x2(x2), y2(y2)
    {
    }
    //  онструктор дл€ хода с рубкой (указываетс€ координата побитой фигуры)
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }
    // ќператор сравнени€: два хода считаютс€ равными, если их начальна€ и конечна€ позиции совпадают
    bool operator==(const move_pos &other) const
    {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2);
    }
    // ќператор неравенства (противоположен оператору ==)
    bool operator!=(const move_pos &other) const
    {
        return !(*this == other);
    }
};
