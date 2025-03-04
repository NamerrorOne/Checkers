#pragma once
#include <chrono>
#include <thread>

#include "../Models/Project_path.h"
#include "Board.h"
#include "Config.h"
#include "Hand.h"
#include "Logic.h"

class Game
{
  public:
    Game() : board(config("WindowSize", "Width"), config("WindowSize", "Hight")), hand(&board), logic(&board, &config), beat_series(0), is_replay(false)
    {
        ofstream fout(project_path + "log.txt", ios_base::trunc);
        fout.close();
    }

    // to start checkers
    int play()
    {
        auto start = chrono::steady_clock::now(); // Засекаем время начала игры
        if (is_replay) // Если это повтор игры (режим реплея), перезагружаем логику, конфигурацию и перерисовываем доску
        {
            logic = Logic(&board, &config); // Пересоздаём объект Logic
            config.reload(); // Перезагружаем настройки из файла settings.json
            board.redraw(); // Перерисовываем доску
        }
        else
        {
            board.start_draw(); // Иначе начинаем новую игру и рисуем доску
        }
        is_replay = false; // Сбрасываем флаг реплея
         
        int turn_num = -1; // Номер хода (начинаем с -1, так как в цикле сразу увеличиваем)
        bool is_quit = false;  // Флаг для выхода из игры
        const int Max_turns = config("Game", "MaxNumTurns"); // Максимальное количество ходов из настроек

        // Основной цикл игры: продолжается, пока не достигнуто максимальное количество ходов
        while (++turn_num < Max_turns)
        {
            beat_series = 0; // Сбрасываем счётчик серии ударов (для шашек, которые бьют несколько фигур подряд)
            logic.find_turns(turn_num % 2); // Находим возможные ходы для текущего игрока (0 — белые, 1 — чёрные)
            if (logic.turns.empty()) // Если ходов нет, игра завершается
                break;
            // Устанавливаем глубину поиска для бота в зависимости от уровня сложности
            logic.Max_depth = config("Bot", string((turn_num % 2) ? "Black" : "White") + string("BotLevel"));
            // Если текущий игрок — человек (не бот), обрабатываем его ход
            if (!config("Bot", string("Is") + string((turn_num % 2) ? "Black" : "White") + string("Bot")))
            {
                auto resp = player_turn(turn_num % 2); // Обрабатываем ход игрока
                if (resp == Response::QUIT) // Если игрок решил выйти
                {
                    is_quit = true;
                    break;
                }
                else if (resp == Response::REPLAY)  // Если игрок решил переиграть
                {
                    is_replay = true;
                    break;
                }
                else if (resp == Response::BACK) // Если игрок решил отменить ход
                {
                    // Отменяем ход, если это возможно
                    if (config("Bot", string("Is") + string((1 - turn_num % 2) ? "Black" : "White") + string("Bot")) &&
                        !beat_series && board.history_mtx.size() > 2)
                    {
                        board.rollback(); // Откатываем ход
                        --turn_num;  // Уменьшаем счётчик ходов
                    }
                    if (!beat_series)
                        --turn_num;

                    board.rollback();  // Откатываем ещё один ход
                    --turn_num; // Уменьшаем счётчик ходов
                    beat_series = 0;  // Сбрасываем счётчик серии ударов
                }
            }
            else
                bot_turn(turn_num % 2);  // Если текущий игрок — бот, обрабатываем его ход
        }
        auto end = chrono::steady_clock::now();  // Засекаем время окончания игры
        ofstream fout(project_path + "log.txt", ios_base::app); // Открываем файл логов для записи
        fout << "Game time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";  // Записываем время игры
        fout.close();

        // Если был запрошен реплей, запускаем игру заново
        if (is_replay)
            return play();
        // Если игрок решил выйти, возвращаем 0
        if (is_quit)
            return 0;
        // Определяем результат игры
        int res = 2; // По умолчанию результат — ничья
        if (turn_num == Max_turns) // Если достигнуто максимальное количество ходов
        {
            res = 0; // Ничья
        }
        else if (turn_num % 2) // Если последний ход был чёрных
        {
            res = 1; // Победа чёрных
        }
        board.show_final(res); // Показываем финальный экран с результатом игры
        auto resp = hand.wait();  // Ожидаем реакции игрока (например, нажатия кнопки)
        // Если игрок решил переиграть, запускаем игру заново
        if (resp == Response::REPLAY)
        {
            is_replay = true;
            return play();
        }
        return res; // Возвращаем результат игры
    }

  private:
     // Функция bot_turn() выполняет ход бота в зависимости от текущего состояния игры.
    // color - цвет бота (0 — белые, 1 — чёрные).
    void bot_turn(const bool color)
    {
        auto start = chrono::steady_clock::now(); // Засекаем время начала хода бота.

        auto delay_ms = config("Bot", "BotDelayMS"); // Получаем задержку перед ходом бота из конфигурации.
        // Создаём новый поток, который выполняет задержку перед ходом бота.
        thread th(SDL_Delay, delay_ms);
        // Находим лучший ход для бота с использованием алгоритма минимакса.
        auto turns = logic.find_best_turns(color);
        // Дожидаемся завершения задержки.
        th.join();
        // Флаг для первого хода в серии.
        bool is_first = true;
        // Выполняем найденный ход (или серию ходов, если возможны дополнительные удары).
        for (auto turn : turns)
        {
            if (!is_first)
            {
                SDL_Delay(delay_ms); // Добавляем задержку перед каждым следующим ходом.
            }
            is_first = false;
            // Если бот выполняет захват шашки, увеличиваем счётчик серии ударов.
            beat_series += (turn.xb != -1);
            // Выполняем ход на игровой доске
            board.move_piece(turn, beat_series);
        }

        auto end = chrono::steady_clock::now(); // Засекаем время завершения хода.
        // Записываем время выполнения хода бота в лог-файл.
        ofstream fout(project_path + "log.txt", ios_base::app);
        fout << "Bot turn time: " << (int)chrono::duration<double, milli>(end - start).count() << " millisec\n";
        fout.close();
    }

// Функция player_turn() обрабатывает ход игрока (человека).
// color - цвет текущего игрока (0 — белые, 1 — чёрные)
// Возвращает Response (например, QUIT, если игрок решил выйти)
    Response player_turn(const bool color)
    {
        // Вектор для хранения доступных ходов
        // return 1 if quit
        vector<pair<POS_T, POS_T>> cells;
        for (auto turn : logic.turns)
        {
            cells.emplace_back(turn.x, turn.y);
        }
        // Подсвечиваем возможные клетки для хода
        board.highlight_cells(cells);
        // Переменная для хранения выбранного хода
        move_pos pos = {-1, -1, -1, -1};
        POS_T x = -1, y = -1;

        // Основной цикл: ждём, пока игрок сделает первый ход
        // trying to make first move
        while (true)
        {
            auto resp = hand.get_cell(); // Получаем клетку, на которую нажал игрок
            // Если игрок выбрал выход, возврат к предыдущему шагу или переигровку
            if (get<0>(resp) != Response::CELL)
                return get<0>(resp);
            pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)}; // Получаем координаты клетки

            bool is_correct = false;
            for (auto turn : logic.turns)
            {
                // Проверяем, является ли выбранная клетка корректной для хода
                if (turn.x == cell.first && turn.y == cell.second)
                {
                    is_correct = true;
                    break;
                }
                if (turn == move_pos{x, y, cell.first, cell.second})
                {
                    pos = turn;
                    break;
                }
            }
            // Если ход корректный, выходим из цикла
            if (pos.x != -1)
                break;
            // Если ход некорректный, снимаем выделение и ждём нового ввода
            if (!is_correct)
            {
                if (x != -1)
                {
                    board.clear_active();
                    board.clear_highlight();
                    board.highlight_cells(cells);
                }
                x = -1;
                y = -1;
                continue;
            }
            // Если ход возможен, выделяем выбранную клетку и подсвечиваем возможные ходы
            x = cell.first;
            y = cell.second;
            board.clear_highlight();
            board.set_active(x, y);
            vector<pair<POS_T, POS_T>> cells2;
            for (auto turn : logic.turns)
            {
                if (turn.x == x && turn.y == y)
                {
                    cells2.emplace_back(turn.x2, turn.y2);
                }
            }
            board.highlight_cells(cells2);
        }
        // Очищаем подсветку и выполняем ход
        board.clear_highlight();
        board.clear_active();
        board.move_piece(pos, pos.xb != -1);
        // Если ход не был рубящим, завершаем функцию
        if (pos.xb == -1)
            return Response::OK;
        // Если ход был рубящим, продолжаем серию ударов
        // continue beating while can
        beat_series = 1;
        while (true)
        {
            logic.find_turns(pos.x2, pos.y2);
            if (!logic.have_beats)
                break;
            // Подсвечиваем возможные следующие шаги
            vector<pair<POS_T, POS_T>> cells;
            for (auto turn : logic.turns)
            {
                cells.emplace_back(turn.x2, turn.y2);
            }
            board.highlight_cells(cells);
            board.set_active(pos.x2, pos.y2);
           
            // Цикл ожидания следующего удара
            // trying to make move
            while (true)
            {
                auto resp = hand.get_cell();
                if (get<0>(resp) != Response::CELL)
                    return get<0>(resp);
                pair<POS_T, POS_T> cell{get<1>(resp), get<2>(resp)};

                bool is_correct = false;
                for (auto turn : logic.turns)
                {
                    if (turn.x2 == cell.first && turn.y2 == cell.second)
                    {
                        is_correct = true;
                        pos = turn;
                        break;
                    }
                }
                if (!is_correct)
                    continue;
                // Выполняем следующий удар
                board.clear_highlight();
                board.clear_active();
                beat_series += 1;
                board.move_piece(pos, beat_series);
                break;
            }
        }

        return Response::OK;
    }

  private:
    Config config;
    Board board;
    Hand hand;
    Logic logic;
    int beat_series;
    bool is_replay = false;
};
