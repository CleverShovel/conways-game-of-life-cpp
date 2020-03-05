#include "game_state.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <algorithm>
#include <array>
#include <utility>
#include <vector>

#include <algorithm>
#include <random>

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
    // размер окна
    sf::RenderWindow window(sf::VideoMode(640, 700), "");

    // window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
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

    vector<RectangleShape> cells;

    window.setTitle(windowTitle);
    sf::Clock deltaClock;
    sf::Clock fpsClock;
    while (window.isOpen()) {
        float fps = 1.f / fpsClock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        gameState.NextState();

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
        ImGui::Text("%d", int(fps));
        ImGui::End();

        auto& state = gameState.GetState();
        for (int i = 0; i < cellColCount; i++) {
            for (int j = 0; j < cellRowCount; j++) {
                if (state[j][i] == CellState::Dead) continue;
                cells.emplace_back(Vector2f(cellWidth, cellHeight));
                cells.back().setPosition(Vector2f(
                    i * cellWidth + zonePos.x,
                    j * cellHeight + zonePos.y));
                cells.back().setFillColor(aliveCellColor);
            }
        }

        // vector<array<sf::Vertex, 2>> lines;
        // for (int i = 1; i < cellColCount; i++) {
        //     lines.push_back({ Vector2f(i * cellWidth + zonePos.x, zonePos.y),
        //         Vector2f(i * cellWidth + zonePos.x, zonePos.y + zoneHeight) });
        //     lines.back()[0].color = zoneColor;
        //     lines.back()[1].color = zoneColor;
        // }

        // for (int i = 1; i < cellRowCount; i++) {
        //     lines.push_back({ Vector2f(zonePos.x, i * cellHeight + zonePos.y),
        //         Vector2f(zonePos.x + zoneWidth, i * cellHeight + zonePos.y) });
        //     lines.back()[0].color = zoneColor;
        //     lines.back()[1].color = zoneColor;
        // }

        window.clear(bgColor); // fill background with color

        window.draw(zone);

        for (const auto& cell : cells)
            window.draw(cell);

        // for (const auto& line : lines)
        //     window.draw(line.data(), 2, sf::Lines);

        ImGui::SFML::Render(window);
        window.display();

        cells.clear();
    }

    ImGui::SFML::Shutdown();
}