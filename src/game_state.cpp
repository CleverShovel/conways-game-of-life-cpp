#include "game_state.h"

#include <array>
#include <utility>

using std::array;
using std::vector;

GameState::GameState(int rowCount, int colCount)
    : rowCount_(rowCount)
    , colCount_(colCount)
    , state_(rowCount, vector(colCount, CellState::Dead))
    , temp_(state_)
{
}

GameState::GameState(StateMatrix state)
    : rowCount_(state.size())
    , colCount_(state.size() > 0 ? state[0].size() : 0)
    , state_(std::move(state))
    , temp_(state_)
{
}

int GameState::CountAliveNeighbours(int x, int y) const
{
    const static auto dx = array{ -1, 0, 1, 1, 1, 0, -1, -1 };
    const static auto dy = array{ -1, -1, -1, 0, 1, 1, 1, 0 };

    int count = 0;

    for (int i = 0; i < dx.size() && count < 4; i++) {
        int new_x = x + dx[i];
        int new_y = y + dy[i];
        count += (new_x >= 0 && new_x < colCount_ && new_y >= 0 && new_y < rowCount_
            && state_[new_y][new_x] == CellState::Alive);
    }

    return count;
}

void GameState::NextState()
{
    for (int i = 0; i < rowCount_; i++) {
        for (int j = 0; j < colCount_; j++) {
            int count = CountAliveNeighbours(j, i);
            temp_[i][j] = static_cast<CellState>(
                count == 3 || count == 2 && state_[i][j] == CellState::Alive);
        }
    }
    state_.swap(temp_);
}

const GameState::StateMatrix& GameState::GetState() const { return state_; }