#include "game_state.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <future>
#include <utility>
#include <vector>

#include <random>

#include <iostream>
#include <iterator>

using sf::RectangleShape;
using sf::Vector2f;
using std::array;
using std::vector;

GameState::StateMatrix GetRandomMatrix(int rowCount, int colCount)
{
    std::random_device r;
    std::default_random_engine e(r());
    std::uniform_int_distribution<int> uniform_dist(0, 1);

    auto matrix = GameState::StateMatrix(rowCount, vector(colCount, CellState::Dead));
    for (auto& v : matrix)
        std::generate(v.begin(), v.end(), [&]() {
            return static_cast<CellState>(uniform_dist(e));
        });
    return matrix;
}

int main()
{
    std::ios::sync_with_stdio(false);

    // размер окна
    sf::RenderWindow window(sf::VideoMode(640, 700), "");

    // window.setFramerateLimit(60);
    // window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);

    char windowTitle[255] = "ImGui + SFML = <3";

    auto bgColor = sf::Color::White;
    auto zoneColor = sf::Color::Black;
    auto aliveCellColor = sf::Color::Blue;
    auto deadCellColor = sf::Color::White;

    // размер зоны (там происходит сама игра)
    const auto [zoneWidth, zoneHeight] = std::pair{ 600.f, 600.f };

    // отступ от края окна
    const auto zonePos = sf::Vector2f{ 5.f, 5.f };

    // толщина линии зоны
    const float zoneThickness = 2.f;

    // количество столбцов и строк соответственно
    const auto [cellColCount, cellRowCount] = std::pair{ 600, 600 };

    // размеры клеток (не трогать)
    const auto [cellWidth, cellHeight] = std::pair{
        zoneWidth / cellColCount, zoneHeight / cellRowCount
    };

    RectangleShape zone(sf::Vector2f(zoneWidth, zoneHeight));
    zone.setOutlineColor(zoneColor);
    zone.setPosition(zonePos);
    zone.setOutlineThickness(zoneThickness);
    zone.setFillColor(deadCellColor);

    bool active = false;

    GameState gameState(GetRandomMatrix(cellRowCount, cellColCount));
    // GameState gameState(cellColCount, cellRowCount);

    auto lam = [cc = cellColCount,
                   cr = cellRowCount,
                   cw = cellWidth,
                   ch = cellHeight,
                   zp = zonePos,
                   acolor = aliveCellColor,
                   &gameState](size_t from, size_t to) {
        // sf::Clock asyncClock;
        vector<RectangleShape> cells;
        auto& state = gameState.GetState();
        for (int i = from; i < to; i++) {
            for (int j = 0; j < cr; j++) {
                if (state[j][i] == CellState::Dead)
                    continue;
                cells.emplace_back(Vector2f(cw, ch));
                cells.back().setPosition(Vector2f(
                    i * cw + zp.x,
                    j * ch + zp.y));
                cells.back().setFillColor(acolor);
            }
        }
        // std::cout << "async       " << asyncClock.getElapsedTime().asMicroseconds() << '\n';
        return cells;
    };

    vector<std::future<vector<RectangleShape>>> futures;

    window.setTitle(windowTitle);

    sf::Clock deltaClock;
    sf::Clock fpsClock;
    // sf::Clock profileClock;

    const int threadCount = 4;

    int max_fps = -1;

    while (window.isOpen()) {
        int fps = int(1.f / fpsClock.restart().asSeconds());
        if (max_fps == -1) max_fps = 0;
        else max_fps = std::max(fps, max_fps);

        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        gameState.NextState();

        size_t part = cellRowCount / threadCount + (cellRowCount % threadCount > 0 ? 1 : 0);
        for (int i = 0; i < threadCount - 1; i++) {
            size_t from = part * i;
            size_t to = from + std::min(cellRowCount - from, part);
            futures.push_back(std::async(
                std::launch::async, lam, from, to));
        }

        vector<vector<RectangleShape>> cells;
        cells.push_back(lam((threadCount - 1)*part, cellRowCount));

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::SetNextWindowPos(ImVec2(zonePos.x, zoneHeight + zonePos.y + 5.f), ImGuiCond_Once);
        ImGui::Begin("State");
        if (ImGui::Button("Start"))
            active = true;
        if (ImGui::Button("Pause"))
            active = false;
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(zonePos.x, zonePos.y));
        ImGui::Begin("FPS");
        ImGui::Text("current: %d", fps);
        ImGui::Text("max:     %d", max_fps);
        ImGui::End();

        window.clear(bgColor); // fill background with color

        window.draw(zone);

        // long long beforeWait = profileClock.getElapsedTime().asMicroseconds();

        for (auto& f : futures) {
            cells.push_back(f.get());
        }

        // long long wait = profileClock.getElapsedTime().asMicroseconds() - beforeWait;

        for (const auto& v : cells)
            for (const auto& cell : v)
                window.draw(cell);

        // long long drawCells = profileClock.getElapsedTime().asMicroseconds() - beforeWait - wait;

        ImGui::SFML::Render(window);
        window.display();

        futures.clear();
        // std::cout << "before wait " << beforeWait << '\n';
        // std::cout << "wait        " << wait << '\n';
        // std::cout << "draw cells  " << drawCells << '\n';
        // std::cout << "main all    " << profileClock.restart().asMicroseconds() << "\n\n";
    }

    ImGui::SFML::Shutdown();
}