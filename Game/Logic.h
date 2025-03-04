#pragma once
#include <random>
#include <vector>

#include "../Models/Move.h"
#include "Board.h"
#include "Config.h"


const double INF = 1e9; // Константа, обозначающая "бесконечность" для алгоритма минимакса

class Logic
{
  public:
      // Конструктор класса, принимает указатели на игровую доску и конфигурацию
    Logic(Board *board, Config *config) : board(board), config(config)
    {
        rand_eng = std::default_random_engine (
            !((*config)("Bot", "NoRandom")) ? unsigned(time(0)) : 0); // Инициализация генератора случайных чисел
        scoring_mode = (*config)("Bot", "BotScoringType"); // Тип оценки ходов (например, на основе количества фигур)
        optimization = (*config)("Bot", "Optimization"); // Уровень оптимизации бота
    }
    // Функция находит лучшие ходы для бота, используя алгоритм минимакса с альфа-бета отсечением.
    vector<move_pos> find_best_turns(const bool color)
    {
        next_best_state.clear(); // Очищаем вектор состояний
        next_move.clear(); // Очищаем вектор ходов

        // Запускаем поиск лучшего хода с начальным состоянием доски
        find_first_best_turn(board->get_board(), color, -1, -1, 0);

        // Формируем список ходов, начиная с корневого состояния
        int cur_state = 0;
        vector<move_pos> res;
        do
        {
            res.push_back(next_move[cur_state]);
            cur_state = next_best_state[cur_state];
        } while (cur_state != -1 && next_move[cur_state].x != -1);

        return res; // Возвращаем лучший найденный ход
    }

    // Функция ищет лучший первый ход для бота, используя минимаксный алгоритм.
    double find_first_best_turn(vector<vector<POS_T>> mtx, const bool color, const POS_T x, const POS_T y, size_t state,
        double alpha = -1)
    {
        next_best_state.push_back(-1); // Добавляем новое состояние с изначальным значением -1
        next_move.emplace_back(-1, -1, -1, -1); // Добавляем фиктивный ход в вектор

        double best_score = -INF; // Инициализируем наихудший возможный счёт

        if (state != 0) // Если это не начальное состояние, находим доступные ходы
            find_turns(x, y, mtx);

        auto turns_now = turns; // Копируем найденные ходы
        bool have_beats_now = have_beats; // Проверяем, есть ли возможность побить шашку

        // Если нет ударов и это не начальное состояние, переключаем ход на другого игрока
        if (!have_beats_now && state != 0)
        {
            return find_best_turns_rec(mtx, !color, 0, alpha);
        }

        // Перебираем все возможные ходы
        for (auto turn : turns_now)
        {
            size_t next_state = next_move.size(); // Запоминаем индекс следующего состояния
            double score; // Оценка хода

            if (have_beats_now) // Если у нас есть возможность побить шашку, продолжаем серию ударов
            {
                score = find_first_best_turn(make_turn(mtx, turn), color, turn.x2, turn.y2, next_state, best_score);
            }
            else // Если нет ударов, передаём ход противнику
            {
                score = find_best_turns_rec(make_turn(mtx, turn), !color, 0, best_score);
            }

            // Если ход лучше предыдущего, обновляем лучшую оценку и лучший ход
            if (score > best_score)
            {
                best_score = score;
                next_best_state[state] = have_beats_now ? int(next_state) : -1;
                next_move[state] = turn;
            }
        }
        return best_score; // Возвращаем оценку лучшего найденного хода
    }

    // Рекурсивная функция минимакса с альфа-бета отсечением.
    // color - чей ход (0 - белые, 1 - чёрные)
    // depth - текущая глубина поиска
    double find_best_turns_rec(vector<vector<POS_T>> mtx, const bool color, const size_t depth, double alpha = -INF,
        double beta = INF, const POS_T x = -1, const POS_T y = -1)
    {
        if (depth == Max_depth) // Если достигли максимальной глубины, оцениваем позицию
        {
            return calc_score(mtx, color);
        }

        if (x != -1) // Если продолжаем серию ударов, проверяем доступные ходы для этой шашки
        {
            find_turns(x, y, mtx);
        }
        else // Иначе ищем все возможные ходы для игрока
        {
            find_turns(color, mtx);
        }

        auto turns_now = turns; // Получаем найденные ходы
        bool have_beats_now = have_beats; // Проверяем, есть ли возможность побить шашку

        if (!have_beats_now && x != -1) // Если удары закончились, передаём ход противнику
        {
            return find_best_turns_rec(mtx, !color, depth + 1, alpha, beta);
        }

        if (turns.empty()) // Если ходов нет, значит это проигрыш
        {
            return (depth % 2 == 0) ? INF : 0;
        }

        // Минимальная и максимальная оценки
        double min_score = INF;
        double max_score = -INF;

        // Перебираем все возможные ходы
        for (auto turn : turns_now)
        {
            double score;
            if (!have_beats_now && x == -1) // Если ход обычный, передаём ход противнику
            {
                score = find_best_turns_rec(make_turn(mtx, turn), !color, depth + 1, alpha, beta);
            }
            else // Если ход с рубкой, продолжаем серию ударов
            {
                score = find_best_turns_rec(make_turn(mtx, turn), color, depth, alpha, beta, turn.x2, turn.y2);
            }

            min_score = min(min_score, score);
            max_score = max(max_score, score);

            // Альфа-бета отсечение
            if (depth % 2 == 0) // Ход бота (максимизирующий игрок)
            {
                alpha = max(alpha, max_score);
            }
            else // Ход противника (минимизирующий игрок)
            {
                beta = min(beta, min_score);
            }

            // Если нашли достаточно хороший ход, прерываем дальнейший поиск
            if (optimization != "O0" && alpha >= beta)
            {
                return (depth % 2 == 0) ? max_score : min_score;
            }
        }

        return (depth % 2 == 0) ? max_score : min_score; // Возвращаем наилучшую найденную оценку
    }

private:
    // Функция выполняет виртуальный ход и возвращает новую копию доски после этого хода
    vector<vector<POS_T>> make_turn(vector<vector<POS_T>> mtx, move_pos turn) const
    {
        if (turn.xb != -1) // Если был захват шашки, удаляем побитую фигуру
            mtx[turn.xb][turn.yb] = 0;
        // Если обычная шашка дошла до конца доски, она становится дамкой
        if ((mtx[turn.x][turn.y] == 1 && turn.x2 == 0) || (mtx[turn.x][turn.y] == 2 && turn.x2 == 7))
            mtx[turn.x][turn.y] += 2;
        // Перемещаем шашку на новую позицию
        mtx[turn.x2][turn.y2] = mtx[turn.x][turn.y];
        // Очищаем предыдущую клетку
        mtx[turn.x][turn.y] = 0;
        return mtx;  // Возвращаем обновлённое состояние доски
    }

    // Функция оценивает текущее состояние доски и возвращает числовой показатель (чем выше, тем лучше для бота)
    double calc_score(const vector<vector<POS_T>> &mtx, const bool first_bot_color) const
    {
        // color - определяет, кто является максимизирующим игроком (бот или противник)
        double w = 0, wq = 0, b = 0, bq = 0;
        // Подсчёт количества шашек и дамок на доске.
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                w += (mtx[i][j] == 1); // Количество белых шашек
                wq += (mtx[i][j] == 3); // Количество белых дамок
                b += (mtx[i][j] == 2); // Количество чёрных шашек
                bq += (mtx[i][j] == 4); // Количество чёрных дамок
                // Если используется метод "NumberAndPotential", учитываем "потенциал" шашек (приближенность к дамке)
                if (scoring_mode == "NumberAndPotential")
                {
                    w += 0.05 * (mtx[i][j] == 1) * (7 - i); // Чем ближе к противоположному краю, тем выше оценка
                    b += 0.05 * (mtx[i][j] == 2) * (i);
                }
            }
        }
        // Если бот играет за чёрных, меняем местами значения
        if (!first_bot_color)
        {
            swap(b, w);
            swap(bq, wq);
        }
        // Если у бота не осталось фигур, это поражение (возвращаем "бесконечность")
        if (w + wq == 0)
            return INF;
        // Если у противника не осталось фигур, это победа (возвращаем 0)
        if (b + bq == 0)
            return 0;
        // Коэффициент значимости дамок (по умолчанию 4, но если учёт потенциала включён — 5)
        int q_coef = 4;
        if (scoring_mode == "NumberAndPotential")
        {
            q_coef = 5;
        }
        // Оцениваем силу позиций: чем выше значение, тем выгоднее текущая позиция для бота
        return (b + bq * q_coef) / (w + wq * q_coef);
    }

   
public:
    // Найти все возможные ходы для заданного цвета (0 — белые, 1 — чёрные)
    void find_turns(const bool color)
    {
        find_turns(color, board->get_board());
    }
    // Найти все возможные ходы для заданной фигуры по её координатам (x, y)
    void find_turns(const POS_T x, const POS_T y)
    {
        find_turns(x, y, board->get_board());
    }

private:
    // Перегруженная функция, находит все возможные ходы для указанного цвета.
    // `color` — цвет игрока (0 — белые, 1 — чёрные).
    // `mtx` — текущее состояние игровой доски.
    void find_turns(const bool color, const vector<vector<POS_T>> &mtx)
    {
        vector<move_pos> res_turns; // Вектор возможных ходов
        bool have_beats_before = false; // Флаг, были ли удары
        // Проход по всей доске для поиска возможных ходов
        for (POS_T i = 0; i < 8; ++i)
        {
            for (POS_T j = 0; j < 8; ++j)
            {
                // Если клетка занята и её цвет совпадает с текущим игроком
                if (mtx[i][j] != 0 && (static_cast<int>(mtx[i][j]) % 2) != static_cast<int>(color))

                {
                    find_turns(i, j, mtx); // Найти все возможные ходы для данной шашки
                    // Если появилась возможность бить шашку и раньше не было ударов — очищаем список ходов
                    if (have_beats && !have_beats_before)
                    {
                        have_beats_before = true;
                        res_turns.clear();
                    }
                    // Добавляем ходы в список
                    if ((have_beats_before && have_beats) || !have_beats_before)
                    {
                        res_turns.insert(res_turns.end(), turns.begin(), turns.end());
                    }
                }
            }
        }
        turns = res_turns; // Обновляем список ходов
        shuffle(turns.begin(), turns.end(), rand_eng); // Перемешиваем ходы (если активирован случайный порядок)
        have_beats = have_beats_before; // Обновляем флаг ударов
    }
   
        // Перегруженная функция, находит возможные ходы для одной фигуры.
       // `x`, `y` — координаты фигуры.
       // `mtx` — текущее состояние игровой доски.
    void find_turns(const POS_T x, const POS_T y, const vector<vector<POS_T>> &mtx)
    {
        turns.clear();  // Очищаем список возможных ходов
        have_beats = false; // Сбрасываем флаг наличия ударов
        POS_T type = mtx[x][y]; // Получаем тип фигуры (обычная шашка или дамка)
        // Проверяем возможность захвата (ударов)
        switch (type)
        {
        case 1: // Белая шашка
        case 2: // Чёрная шашка
            // check pieces
            for (POS_T i = x - 2; i <= x + 2; i += 4) // Проверка диагоналей
            {
                for (POS_T j = y - 2; j <= y + 2; j += 4)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7) // Проверяем границы доски
                        continue;
                    POS_T xb = (x + i) / 2, yb = (y + j) / 2; // Координаты возможной побитой шашки
                    // Проверяем, можно ли сделать удар
                    if (mtx[i][j] || !mtx[xb][yb] || mtx[xb][yb] % 2 == type % 2)  
                        continue;
                    turns.emplace_back(x, y, i, j, xb, yb);  // Добавляем удар в список возможных ходов
                }
            }
            break;
        default:  // Дамка (белая или чёрная)
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    POS_T xb = -1, yb = -1;
                    // Дамка может двигаться по диагонали на любое количество клеток
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                        {
                            if (mtx[i2][j2] % 2 == type % 2 || (mtx[i2][j2] % 2 != type % 2 && xb != -1))
                            {
                                break;
                            }
                            xb = i2;
                            yb = j2;
                        }
                        if (xb != -1 && xb != i2)
                        {
                            turns.emplace_back(x, y, i2, j2, xb, yb);  // Добавляем возможный ход дамки
                        }
                    }
                }
            }
            break;
        }
        // Если были найдены удары, останавливаем дальнейший поиск
        if (!turns.empty())
        {
            have_beats = true;
            return;
        }
        // Если ударов нет, проверяем обычные ходы
        switch (type)
        {
        case 1: // Белая шашка
        case 2: // Чёрная шашка
            // check pieces
            {
                POS_T i = ((type % 2) ? x - 1 : x + 1); // Вычисляем направление хода
                for (POS_T j = y - 1; j <= y + 1; j += 2)
                {
                    if (i < 0 || i > 7 || j < 0 || j > 7 || mtx[i][j])
                        continue;
                    turns.emplace_back(x, y, i, j);  // Добавляем обычный ход
                }
                break;
            }
        default: // Дамка
            // check queens
            for (POS_T i = -1; i <= 1; i += 2)
            {
                for (POS_T j = -1; j <= 1; j += 2)
                {
                    for (POS_T i2 = x + i, j2 = y + j; i2 != 8 && j2 != 8 && i2 != -1 && j2 != -1; i2 += i, j2 += j)
                    {
                        if (mtx[i2][j2])
                            break;
                        turns.emplace_back(x, y, i2, j2); // Добавляем возможный ход дамки
                    }
                }
            }
            break;
        }
    }

  public:
    vector<move_pos> turns; // Список возможных ходов
    bool have_beats; // Флаг наличия ударов (если true, шашки могут бить)
    int Max_depth; // Максимальная глубина поиска для алгоритма минимакса

  private:
    default_random_engine rand_eng; // Генератор случайных чисел (для случайного выбора ходов бота)
    string scoring_mode; // Метод оценки позиции (например, "NumberAndPotential")
    string optimization;  // Оптимизационные параметры для алгоритма поиска
    vector<move_pos> next_move;  // Ходы, выбранные ботом на каждом уровне поиска
    vector<int> next_best_state; // Следующие состояния после ходов
    Board *board;  // Указатель на объект игрового поля
    Config *config; // Указатель на объект с настройками игры
};
