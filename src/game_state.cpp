#include "game_state.h"

#include <algorithm>
#include <array>
#include <future>
#include <random>
#include <utility>

using namespace std;

GameState::StateMatrix GenerateMatrix(int rowCount, int colCount)
{
    random_device r;
    default_random_engine e(r());
    uniform_int_distribution<int> uniform_dist(0, 1);

    auto matrix = GameState::StateMatrix(rowCount * colCount, CellState::Dead);
    generate(matrix.begin(), matrix.end(), [&]() {
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
    SetFromTos();
}

GameState::GameState(StateMatrix state, int rowCount, int colCount)
    : rowCount_(rowCount)
    , colCount_(colCount)
    , state_(std::move(state))
    , temp_(state_)
{
    SetFromTos();
}

void GameState::NextStateSeq(int from, int to)
{
    for (int i = from; i < to; i++) {
        int count = CountAliveNeighbours(i % colCount_, i / colCount_);
        temp_[i] = static_cast<CellState>(
            count == 3 || count == 2 && state_[i] == CellState::Alive);
    }
}

void GameState::NextState()
{
    if (!active_)
        return;

    tf::Taskflow tf;

    auto [S, T] = tf.parallel_for(
        0, int(state_.size()), 1, [&](int i) {
            int count = CountAliveNeighbours(i % colCount_, i / colCount_);
            temp_[i] = static_cast<CellState>(
                count == 3 || count == 2 && state_[i] == CellState::Alive);
        },
        5000);

    exec_.run(tf).wait();

    // vector<future<void>> futs;
    // for (int i = 0; i < fromTos_.size() - 1; i++) {
    //     futs.push_back(async(
    //         launch::async, &GameState::NextStateSeq, this, fromTos_[i].first, fromTos_[i].second));
    // }
    // NextStateSeq(fromTos_[fromTos_.size() - 1].first, fromTos_[fromTos_.size() - 1].second);

    // futs.clear();

    // NextStateSeq(0, state_.size());

    state_.swap(temp_);

    generation_++;
}

const GameState::StateMatrix& GameState::GetState() const { return state_; }

void GameState::SetState(GameState::StateMatrix state, int rowCount, int colCount)
{
    state_.swap(state);
    rowCount_ = rowCount;
    colCount_ = colCount;
    generation_ = 0;
    temp_.resize(state_.size());
    temp_.shrink_to_fit();
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

const vector<pair<int, int>>& GameState::GetFromTos() const
{
    return fromTos_;
}

int GameState::CountAliveNeighbours(int x, int y) const
{
    constexpr static auto dx = array { -1, 0, 1, 1, 1, 0, -1, -1 };
    constexpr static auto dy = array { -1, -1, -1, 0, 1, 1, 1, 0 };

    int count = 0;

    for (size_t i = 0; i < dx.size(); i++) {
        int new_x = x + dx[i];
        int new_y = y + dy[i];
        count += (clamp(new_x, 0, colCount_ - 1) == new_x
            && clamp(new_y, 0, rowCount_ - 1) == new_y
            && bool(state_[new_y * colCount_ + new_x]));
    }

    return count;
}

void GameState::SetFromTos()
{
    int fromTosSize = int(state_.size()) / taskSize + (int(state_.size()) % taskSize > 0);
    fromTos_.resize(fromTosSize);
    for (int i = 0; i < fromTosSize - 1; i++) {
        fromTos_[i] = { taskSize * i, taskSize * (i + 1) };
    }
    fromTos_[fromTosSize - 1] = { taskSize * (fromTosSize - 1), state_.size() };
}