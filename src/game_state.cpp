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

GameState::GameState(int rowCount, int colCount, int threadCount)
    : rowCount_(rowCount)
    , colCount_(colCount)
    , state_(GenerateMatrix(rowCount_, colCount_))
    , temp_(state_)
    , threadCount_(threadCount)
    , fromTos_(threadCount_ * 2)
{
    SetFromTos();
}

GameState::GameState(StateMatrix state, int threadCount)
    : rowCount_(state.size())
    , colCount_(state.size() > 0 ? state[0].size() : 0)
    , state_(std::move(state))
    , temp_(state_)
    , threadCount_(threadCount_)
    , fromTos_(threadCount_ * 2)
{
    SetFromTos();
}

void GameState::NextStateSeq(int from, int to)
{
    for (int i = from; i < to; i++) {
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
    std::vector<std::future<void>> futs;
    for (int i = 0; i < threadCount_ - 1; i++) {
        futs.push_back(std::async(
            std::launch::async, &GameState::NextStateSeq, this, fromTos_[i * 2], fromTos_[i * 2 + 1]));
    }
    NextStateSeq(fromTos_[(threadCount_ - 1) * 2], fromTos_[threadCount_ * 2 - 1]);

    futs.clear();

    state_.swap(temp_);

    generation_++;
}

const GameState::StateMatrix& GameState::GetState() const { return state_; }

void GameState::SetState(GameState::StateMatrix state)
{
    state_.swap(state);
    rowCount_ = state_.size();
    colCount_ = (state_.empty() ? 0 : state_[0].size());
    generation_ = 0;
    temp_ = state_;
    SetFromTos();
}

void GameState::Restart()
{
    GenerateMatrix(rowCount_, colCount_).swap(state_);
    generation_ = 0;
}

void GameState::Pause() { active_ = false; }

void GameState::Unpause() { active_ = true; }

int GameState::GetGeneration() const { return generation_; }

const std::vector<int>& GameState::GetFromTos() const
{
    return fromTos_;
}

int GameState::CountAliveNeighbours(int x, int y) const
{
    const static auto dx = array{ -1, 0, 1, 1, 1, 0, -1, -1 };
    const static auto dy = array{ -1, -1, -1, 0, 1, 1, 1, 0 };

    int count = 0;

    for (size_t i = 0; i < dx.size(); i++) {
        int new_x = x + dx[i];
        int new_y = y + dy[i];
        count += (std::clamp(new_x, 0, colCount_ - 1) == new_x
            && std::clamp(new_y, 0, rowCount_ - 1) == new_y
            && state_[new_y][new_x] == CellState::Alive);
    }

    return count;
}

void GameState::SetFromTos()
{
    int part = rowCount_ / threadCount_ + (rowCount_ % threadCount_ > 0 ? 1 : 0);
    for (int i = 0; i < threadCount_ - 1; i++) {
        fromTos_[i * 2] = part * i;
        fromTos_[i * 2 + 1] = part * (i + 1);
    }
    fromTos_[(threadCount_ - 1) * 2] = part * (threadCount_ - 1);
    fromTos_[threadCount_ * 2 - 1] = rowCount_;
}