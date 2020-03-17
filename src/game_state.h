#pragma once

#include <taskflow/taskflow.hpp>

#include <vector>
#include <utility>

enum class CellState {
    Dead,
    Alive
};

class GameState {
public:
    using StateMatrix = std::vector<CellState>;

    GameState(int rowCount, int colCount);
    GameState(StateMatrix state, int rowCount, int colCount);
    void NextState();
    void NextStateSeq(int begin, int end);
    const StateMatrix& GetState() const;
    void SetState(StateMatrix state, int rowCount, int colCount);
    void Restart();
    void Pause();
    void Unpause();
    int GetGeneration() const;
    const std::vector<std::pair<int, int>>& GetFromTos() const;

private:
    int rowCount_, colCount_;
    StateMatrix state_, temp_;
    bool active_ = true;
    int generation_ = 0;

    tf::Executor exec_;

    constexpr static int taskSize = 60000;
    std::vector<std::pair<int, int>> fromTos_;

    int CountAliveNeighbours(int x, int y) const;
    void SetFromTos();
};