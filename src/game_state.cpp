#include "game_state.h"

#include <array>
#include <utility>

using std::array;
using std::vector;

GameState::GameState(int rowCount, int colCount)
    : rowCount_(rowCount)
    , colCount_(colCount)
    , state_(rowCount, vector(colCount, CellState::Dead))
{
}

GameState::GameState(StateMatrix state)
    : rowCount_(state.size())
    , colCount_(state.size() > 0 ? state[0].size() : 0)
    , state_(std::move(state))
{
}

int GameState::CountAliveNeighbours(int x, int y) const
{
    const auto dx = array{ -1, 0, 1, 1, 1, 0, -1, -1 };
    const auto dy = array{ -1, -1, -1, 0, 1, 1, 1, 0 };

    int count = 0;

    for (int i = 0; i < dx.size(); i++) {
        int new_x = x + dx[i];
        int new_y = y + dy[i];
        if (new_x >= 0 && new_x < colCount_ && new_y >= 0 && new_y < rowCount_
            && state_[new_y][new_x] == CellState::Alive)
            count++;
    }

    return count;
}

void GameState::NextState()
{
    auto newState = vector(rowCount_, vector(colCount_, CellState::Dead));
    for (int i = 0; i < rowCount_; i++) {
        for (int j = 0; j < colCount_; j++) {
            int count = CountAliveNeighbours(j, i);
            if (count == 3 && state_[i][j] == CellState::Dead
                || (count == 2 || count == 3) && state_[i][j] == CellState::Alive)
                newState[i][j] = CellState::Alive;
            else
                newState[i][j] = CellState::Dead;
        }
    }
    state_.swap(newState);
}

const GameState::StateMatrix& GameState::GetState() const { return state_; }