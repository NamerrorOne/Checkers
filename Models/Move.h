#pragma once
#include <stdlib.h>

typedef int8_t POS_T; // Определяем POS_T как 8-битный целочисленный тип для хранения координат

// Структура move_pos описывает ход в игре
struct move_pos
{
    POS_T x, y;              // Координаты начальной позиции фигуры (откуда)
    POS_T x2, y2;            // Координаты конечной позиции фигуры (куда)
    POS_T xb = -1, yb = -1; // Координаты побитой фигуры (если ход с рубкой, иначе -1)

    // Конструктор для простого хода (без рубки)
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2) : x(x), y(y), x2(x2), y2(y2)
    {
    }
    // Конструктор для хода с рубкой (указывается координата побитой фигуры)
    move_pos(const POS_T x, const POS_T y, const POS_T x2, const POS_T y2, const POS_T xb, const POS_T yb)
        : x(x), y(y), x2(x2), y2(y2), xb(xb), yb(yb)
    {
    }
    // Оператор сравнения: два хода считаются равными, если их начальная и конечная позиции совпадают
    bool operator==(const move_pos& other) const
    {
        return (x == other.x && y == other.y && x2 == other.x2 && y2 == other.y2);
    }
    // Оператор неравенства (противоположен оператору ==)
    bool operator!=(const move_pos& other) const
    {
        return !(*this == other);
    }
};
