#pragma once

#include <vector>

enum class CellState {
    Dead,
    Alive
};

class GameState {
public:
    using StateMatrix = std::vector<CellState>;

    GameState(int rowCount, int colCount, int threadCount);
    GameState(StateMatrix state, int rowCount, int colCount, int threadCount);
    void NextState();
    void NextStateSeq(int begin, int end);
    const StateMatrix& GetState() const;
    void SetState(StateMatrix state, int rowCount, int colCount);
    void Restart();
    void Pause();
    void Unpause();
    int GetGeneration() const;
    const std::vector<int>& GetFromTos() const;

private:
    int rowCount_, colCount_;
    StateMatrix state_, temp_;
    bool active_ = true;
    int generation_ = 0;
    const int threadCount_;
    std::vector<int> fromTos_;

    int CountAliveNeighbours(int x, int y) const;
    void SetFromTos();
};