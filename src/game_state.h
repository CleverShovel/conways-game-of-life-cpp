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
    bool active_ = true;
    long long generation_ = 0;

    int CountAliveNeighbours(int x, int y) const;

public:
    GameState(int rowCount, int colCount);
    GameState(StateMatrix state);
    void NextState();
    void NextStateSeq(int begin, int end);
    const StateMatrix& GetState() const;
    void SetState(StateMatrix state);
    void Restart();
    void Pause();
    void Unpause();
    long long GetGeneration() const;
};