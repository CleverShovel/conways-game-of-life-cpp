#include "game_state.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <array>
#include <cmath>
#include <future>
#include <utility>
#include <vector>

// #include <iostream>

using sf::Vector2f;
using sf::Vertex;
using std::array;
using std::vector;

int main(int argc, char* argv[])
{
    int max_generation = (argc > 1 ? atoi(argv[1]) : -1);
    const int threadCount = std::max(2, int(std::thread::hardware_concurrency()));

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

    sf::RectangleShape zone(sf::Vector2f(zoneWidth, zoneHeight));
    zone.setOutlineColor(zoneColor);
    zone.setPosition(zonePos);
    zone.setOutlineThickness(zoneThickness);
    zone.setFillColor(deadCellColor);

    GameState gameState(cellColCount, cellRowCount, threadCount);

    auto lam = [cc = cellColCount,
                   cr = cellRowCount,
                   cw = cellWidth,
                   ch = cellHeight,
                   zp = zonePos,
                   acolor = aliveCellColor,
                   &gameState](int from, int to) {
        sf::VertexArray cells(sf::Triangles);

        auto& state = gameState.GetState();
        for (int i = from; i < to; i++) {
            for (int j = 0; j < cc; j++) {
                if (state[i][j] == CellState::Dead)
                    continue;
                cells.append(Vertex(Vector2f(j * cw + zp.x, i * ch + zp.y), acolor));
                cells.append(Vertex(Vector2f(j * cw + zp.x, i * ch + zp.y + ch), acolor));
                cells.append(Vertex(Vector2f(j * cw + zp.x + cw, i * ch + zp.y + ch), acolor));
                cells.append(Vertex(Vector2f(j * cw + zp.x, i * ch + zp.y + ch), acolor));
                cells.append(Vertex(Vector2f(j * cw + zp.x + cw, i * ch + zp.y + ch), acolor));
                cells.append(Vertex(Vector2f(j * cw + zp.x + cw, i * ch + zp.y), acolor));
            }
        }
        return cells;
    };

    vector<std::future<sf::VertexArray>> futures;
    vector<sf::VertexArray> cells;

    window.setTitle(windowTitle);

    sf::Clock deltaClock;
    sf::Clock fpsClock;

    int maxFPS = -1;

    while (window.isOpen()) {
        int fps = int(1.f / fpsClock.restart().asSeconds());
        if (maxFPS != -1)
            maxFPS = std::max(fps, maxFPS);
        else
            maxFPS = 0;

        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        if (gameState.GetGeneration() == max_generation)
            window.close();

        gameState.NextState();

        auto& fromTos = gameState.GetFromTos();
        for (int i = 0; i < threadCount - 1; i++) {
            futures.push_back(std::async(
                std::launch::async, lam, fromTos[i * 2], fromTos[i * 2 + 1]));
        }

        cells.push_back(lam(fromTos[(threadCount - 1) * 2], fromTos[threadCount * 2 - 1]));

        ImGui::SFML::Update(window, deltaClock.restart());

        // ImGui::ShowMetricsWindow();

        ImGui::Begin("Menu");

        // ImGui::BeginTabBar("Menu");

        // if (ImGui::BeginTabItem("Control")) {
        if (ImGui::Button("Start"))
            gameState.Unpause();
        if (ImGui::Button("Pause"))
            gameState.Pause();
        if (ImGui::Button("Restart"))
            gameState.Restart();
        //     ImGui::EndTabItem();
        // }

        // if (ImGui::BeginTabItem("Settings")) {
        //     ImGui::SliderInt("thread count", &threadCount, 1, maxThreadCount);
        //     ImGui::EndTabItem();
        // }

        // ImGui::EndTabBar();

        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(zonePos.x, zonePos.y), ImGuiCond_Once);
        ImGui::Begin("FPS");
        ImGui::Text("current: %d", fps);
        ImGui::Text("max:     %d", maxFPS);
        ImGui::End();

        ImGui::Begin("Population");
        ImGui::Text("Generation: %d", gameState.GetGeneration());
        ImGui::End();

        window.clear(bgColor);

        window.draw(zone);

        for (auto& f : futures)
            cells.push_back(f.get());

        for (const auto& va : cells)
            window.draw(va);

        ImGui::SFML::Render(window);
        window.display();

        futures.clear();
        cells.clear();
    }

    ImGui::SFML::Shutdown();
}