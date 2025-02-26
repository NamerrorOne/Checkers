#pragma once
#include <SDL.h>

#include <tuple>

#include "../Models/Move.h"
#include "../Models/Response.h"
#include "Board.h"

// Класс Hand обрабатывает ввод пользователя (клики мыши) и управляет взаимодействием с доской
class Hand
{
  public:
     // Конструктор Hand: принимает указатель на объект Board
    Hand(Board *board) : board(board)
    {
    }
    // Метод get_cell() ожидает, пока игрок кликнет на игровое пол и возвращает результат в виде кортежа: {ответ системы, координаты x и y}
    tuple<Response, POS_T, POS_T> get_cell() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK; // Стандартный ответ (если клик некорректен)
        int x = -1, y = -1; // Координаты клика в пикселях
        int xc = -1, yc = -1; // Координаты клика в логической системе координат (ячейки доски)
        while (true)
        {
            if (SDL_PollEvent(&windowEvent)) // Проверяем события в очереди SDL
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT:
                    resp = Response::QUIT; // Игрок закрыл окно, завершаем игру
                    break;
                case SDL_MOUSEBUTTONDOWN: // Игрок кликнул мышью
                    x = windowEvent.motion.x; // Получаем координаты клика по X
                    y = windowEvent.motion.y; // Получаем координаты клика по Y
                    // Преобразуем координаты из пикселей в систему координат доски
                    xc = int(y / (board->H / 10) - 1);
                    yc = int(x / (board->W / 10) - 1);
                    // Если клик был за пределами игрового поля
                    if (xc == -1 && yc == -1 && board->history_mtx.size() > 1)
                    {
                        resp = Response::BACK; // Игрок хочет отменить ход
                    }
                    else if (xc == -1 && yc == 8)
                    {
                        resp = Response::REPLAY; // Игрок хочет переиграть
                    }
                    else if (xc >= 0 && xc < 8 && yc >= 0 && yc < 8)
                    {
                        resp = Response::CELL; // Игрок выбрал клетку на доске
                    }
                    else
                    {
                        xc = -1;
                        yc = -1; // Недопустимый клик, игнорируем его
                    }
                    break;
                case SDL_WINDOWEVENT:
                    if (windowEvent.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        board->reset_window_size(); // Если размер окна изменился, пересчитываем размеры доски
                        break;
                    }
                }
                if (resp != Response::OK)  // Если получен корректный ответ, выходим из цикла
                    break;
            }
        }
        return {resp, xc, yc}; // Возвращаем результат ввода пользователя
    }

    // Метод wait() ожидает действия пользователя (клик или закрытие окна)
   // Возвращает один из возможных ответов Response (QUIT, REPLAY)
    Response wait() const
    {
        SDL_Event windowEvent;
        Response resp = Response::OK; // Стандартный ответ
        while (true)
        {
            if (SDL_PollEvent(&windowEvent))  // Проверяем события в очереди SDL
            {
                switch (windowEvent.type)
                {
                case SDL_QUIT:
                    resp = Response::QUIT;  // Игрок закрыл окно, завершаем игру
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    board->reset_window_size(); // Если изменился размер окна, пересчитываем размеры доски
                    break;
                case SDL_MOUSEBUTTONDOWN: { // Если игрок кликнул мышью
                    int x = windowEvent.motion.x;
                    int y = windowEvent.motion.y;
                    int xc = int(y / (board->H / 10) - 1);
                    int yc = int(x / (board->W / 10) - 1);
                    if (xc == -1 && yc == 8) // Если клик в зоне "переиграть"
                        resp = Response::REPLAY;
                }
                break;
                }
                if (resp != Response::OK) // Если есть действие игрока, завершаем ожидание
                    break;
            }
        }
        return resp; // Возвращаем результат ввода игрока
    }

  private: 
    Board *board; // Указатель на объект доски (Board), с которым взаимодействует игрок
}; 
