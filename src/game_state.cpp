#include "game_state.h"

#include <algorithm>
#include <array>
#include <future>
#include <random>
#include <utility>

using std::array;
using std::vector;

GameState::StateMatrix GenerateMatrix(int rowCount, int colCount)
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

GameState::GameState(int rowCount, int colCount)
    : rowCount_(rowCount)
    , colCount_(colCount)
    , state_(GenerateMatrix(rowCount_, colCount_))
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

    for (size_t i = 0; i < dx.size(); i++) {
        int new_x = x + dx[i];
        int new_y = y + dy[i];
        count += (new_x >= 0 && new_x < colCount_ && new_y >= 0 && new_y < rowCount_
            && state_[new_y][new_x] == CellState::Alive);
    }

    return count;
}

void GameState::NextStateSeq(int begin, int end)
{
    for (int i = begin; i < end; i++) {
        for (int j = 0; j < colCount_; j++) {
            int count = CountAliveNeighbours(j, i);
            temp_[i][j] = static_cast<CellState>(
                count == 3 || count == 2 && state_[i][j] == CellState::Alive);
        }
    }
}

void GameState::NextState()
{
    if (!active_)
        return;
    auto f = std::async(std::launch::async, &GameState::NextStateSeq, this, 0, colCount_ / 2);
    NextStateSeq(colCount_ / 2, colCount_);
    f.get();

    state_.swap(temp_);

    generation_++;
}

const GameState::StateMatrix& GameState::GetState() const { return state_; }

void GameState::SetState(GameState::StateMatrix state) { state_.swap(state); }

void GameState::Restart()
{
    GenerateMatrix(rowCount_, colCount_).swap(state_);
}

void GameState::Pause() { active_ = false; }

void GameState::Unpause() { active_ = true; }

long long GameState::GetGeneration() const { return generation_; }