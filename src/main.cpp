#include "custom_iterators.h"
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

#include <execution>
#include <iterator>

#include <range/v3/range/access.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/cartesian_product.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/for_each.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/transform.hpp>

#include <iostream>

using namespace ranges;

using sf::Vector2f;
using sf::Vertex;
using std::array;
using std::get;
using std::vector;

// TODO сделать реализацию vector для vertex

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
    auto aliveColor = sf::Color::Blue;
    auto deadColor = sf::Color::White;

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

    const static auto vector2fs = array{
        sf::Vector2f(zonePos.x, zonePos.y),
        sf::Vector2f(zonePos.x, zonePos.y + cellHeight),
        sf::Vector2f(zonePos.x + cellWidth, zonePos.y + cellHeight),
        sf::Vector2f(zonePos.x, zonePos.y + cellHeight),
        sf::Vector2f(zonePos.x + cellWidth, zonePos.y + cellHeight),
        sf::Vector2f(zonePos.x + cellWidth, zonePos.y)
    };

    sf::RectangleShape zone(sf::Vector2f(zoneWidth, zoneHeight));
    zone.setOutlineColor(zoneColor);
    zone.setPosition(zonePos);
    zone.setOutlineThickness(zoneThickness);
    zone.setFillColor(deadColor);

    GameState gameState(cellColCount, cellRowCount, threadCount);

    // auto lam = [cc = cellColCount,
    //                cw = cellWidth,
    //                ch = cellHeight,
    //                zp = zonePos,
    //                acolor = aliveColor](const auto& cells_line) {
    //     sf::VertexArray cells(sf::Triangles);

    //     auto& state = gameState.GetState();
    //         for (int j = 0; j < cc; j++) {
    //             if (state[i][j] == CellState::Dead)
    //                 continue;
    //             cells.append(Vertex(Vector2f(j * cw + zp.x, i * ch + zp.y), acolor));
    //             cells.append(Vertex(Vector2f(j * cw + zp.x, i * ch + zp.y + ch), acolor));
    //             cells.append(Vertex(Vector2f(j * cw + zp.x + cw, i * ch + zp.y + ch), acolor));
    //             cells.append(Vertex(Vector2f(j * cw + zp.x, i * ch + zp.y + ch), acolor));
    //             cells.append(Vertex(Vector2f(j * cw + zp.x + cw, i * ch + zp.y + ch), acolor));
    //             cells.append(Vertex(Vector2f(j * cw + zp.x + cw, i * ch + zp.y), acolor));
    //         }
    //     return cells;
    // };

    // vector<std::future<sf::VertexArray>> futures;

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

        auto& state = gameState.GetState();

        auto triangle_idxs = views::cartesian_product(views::cartesian_product(
                                                          views::ints(0, cellColCount),
                                                          views::ints(0, cellRowCount))
                                     | views::filter(
                                           [&](auto tpl) { return bool(state[get<1>(tpl) * cellColCount + get<0>(tpl)]); }),
                                 views::ints(0, 6))
            | to<vector>();

        // auto triangle_idxs = views::cartesian_product(
        //                          views::ints(0, cellColCount),
        //                          views::ints(0, cellRowCount),
        //                          views::ints(0, 6))
        //     | views::filter(
        //           [&](auto tpl) { return bool(state[get<1>(tpl) * cellColCount + get<0>(tpl)]); })
        //     | to<vector>();

        // auto cells
        //     = views::for_each(views::ints(0, cellRowCount * cellColCount)
        //               | views::filter([&](int pos) { return bool(state[pos]); }),
        //           [](int pos) {
        //               return yield_from(
        //                   views::ints(0, 6)
        //                   | views::transform([=](int add) { return (pos << 3) + add; }));
        //           })
        //     | to<vector>();

        vector<Vertex> triangles(triangle_idxs.size());
        std::transform(std::execution::par_unseq, begin(triangle_idxs), end(triangle_idxs), triangles.begin(), [&](const auto& idx) -> Vertex {
            return { Vector2f(
                         get<0>(get<0>(idx)) * cellWidth,
                         get<1>(get<0>(idx)) * cellHeight)
                    + vector2fs[get<1>(idx)],
                aliveColor };
        });

        // auto cells = ints(0, cellRowCount * cellColCount * 6)
        //     | filter([&](int pos) { return state[pos / 6] == CellState::Alive; })
        //     | transform([&](int pos) -> Vertex {
        //           return { Vector2f(
        //                        ((pos / 6) / cellColCount) * cellWidth,
        //                        ((pos / 6) % cellColCount) * cellHeight)
        //                   + vector2fs[pos % 6],
        //               aliveColor };
        //       })
        //     | ranges::to<vector>();
        // std::transform(std::execution::par_unseq, state.begin(), state.end(), lam);

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
        ImGui::Text("generation: %d", gameState.GetGeneration());
        ImGui::End();

        window.clear(bgColor);

        window.draw(zone);

        window.draw(triangles.data(), triangles.size(), sf::Triangles);
        // for (const auto& va : cells)
        //     window.draw(va);

        ImGui::SFML::Render(window);
        window.display();

        // futures.clear();
        // cells.clear();
    }

    ImGui::SFML::Shutdown();
}