#include "game_state.h"

#include <imgui-SFML.h>
#include <imgui.h>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <future>
#include <unordered_map>
#include <utility>
#include <vector>

// #include <iostream>

using namespace std;

using sf::Vector2f;
using sf::Vertex;

// TODO написать класс содержащий указатель на N (шаблонный параметр) подряд идущих объектов для удобного разделения с помощью std::partition
// TODO bidirectional итераторы?

struct QuadHelper {
public:
    QuadHelper(size_t cellPos, sf::Vertex* first)
        : cellPos_(cellPos)
        , first_(first)
    {
    }

    void swap(QuadHelper& other)
    {
        auto tempPos = cellPos_;
        cellPos_ = other.cellPos_;
        other.cellPos_ = tempPos;
        for (size_t i = 0; i < QuadSize; i++) {
            auto temp = first_[i];
            first_[i] = other.first_[i];
            other.first_[i] = temp;
        }
    }

    size_t Pos() const
    {
        return cellPos_;
    }

    static const size_t QuadSize = 6;

private:
    size_t cellPos_;
    sf::Vertex* first_;
};

void swap(QuadHelper& q1, QuadHelper& q2)
{
    q1.swap(q2);
}

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
    const auto [zoneWidth, zoneHeight] = std::pair { 600.f, 600.f };

    // отступ от края окна
    const auto zonePos = sf::Vector2f { 5.f, 5.f };

    // толщина линии зоны
    const float zoneThickness = 2.f;

    // количество столбцов и строк соответственно
    const auto [cellColCount, cellRowCount] = std::pair { 600, 600 };

    // размеры клеток (не трогать)
    const auto [cellWidth, cellHeight] = std::pair {
        zoneWidth / cellColCount, zoneHeight / cellRowCount
    };

    const static auto vector2fs = array {
        Vector2f(zonePos.x, zonePos.y),
        Vector2f(zonePos.x, zonePos.y + cellHeight),
        Vector2f(zonePos.x + cellWidth, zonePos.y + cellHeight),
        Vector2f(zonePos.x, zonePos.y + cellHeight),
        Vector2f(zonePos.x + cellWidth, zonePos.y + cellHeight),
        Vector2f(zonePos.x + cellWidth, zonePos.y)
    };

    sf::RectangleShape zone(sf::Vector2f(zoneWidth, zoneHeight));
    zone.setOutlineColor(zoneColor);
    zone.setPosition(zonePos);
    zone.setOutlineThickness(zoneThickness);
    zone.setFillColor(deadColor);

    GameState gameState(cellColCount, cellRowCount);

    vector<Vertex> triangle_idxs;
    triangle_idxs.reserve(cellRowCount * cellColCount * 6);
    for (size_t i = 0; i < cellRowCount; i++) {
        for (size_t j = 0; j < cellColCount; j++)
            for (size_t k = 0; k < 6; k++)
                triangle_idxs.emplace_back(Vector2f(
                                               j * cellWidth,
                                               i * cellHeight)
                        + vector2fs[k],
                    aliveColor);
    }

    vector<QuadHelper> quads;
    quads.reserve(cellColCount * cellRowCount);
    for (size_t i = 0; i < cellColCount * cellRowCount; i++)
        quads.emplace_back(i, triangle_idxs.data() + i * QuadHelper::QuadSize);

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

        auto partition_size = (partition(quads.begin(), quads.end(),
                                   [&](const auto& quad) { return bool(state[quad.Pos()]); })
                                  - quads.begin())
            * 6;

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

        window.draw(triangle_idxs.data(), partition_size, sf::Triangles);

        ImGui::SFML::Render(window);
        window.display();

        // futures.clear();
        // cells.clear();
    }

    ImGui::SFML::Shutdown();
}