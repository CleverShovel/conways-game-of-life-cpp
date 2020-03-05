#include <vector>

enum class CellState {
    Dead,
    Alive
};

class GameState {
public:
    using StateMatrix = std::vector<std::vector<CellState>>;

private:
    int rowCount_, colCount_;
    StateMatrix state_, temp_;

    int CountAliveNeighbours(int x, int y) const;

public:
    GameState(int rowCount, int colCount);
    GameState(StateMatrix state);
    void NextState();
    const StateMatrix& GetState() const;
};