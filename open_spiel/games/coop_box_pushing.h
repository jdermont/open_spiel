// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef THIRD_PARTY_OPEN_SPIEL_GAMES_COOP_BOX_PUSHING_SOCCER_H_
#define THIRD_PARTY_OPEN_SPIEL_GAMES_COOP_BOX_PUSHING_SOCCER_H_

#include <array>
#include <string>
#include <vector>

#include "open_spiel/simultaneous_move_game.h"
#include "open_spiel/spiel.h"

// This is the cooperative box-pushing domain presented by Seuken & Zilberstein
// in their paper "Improved Memory-Bounded Dynamic Programming for Dec-POMDPs"
// http://rbr.cs.umass.edu/papers/SZuai07.pdf
//
// Parameters:
//     "horizon"       int    length of horizon                (default = 100)

namespace open_spiel {
namespace coop_box_pushing {

// To indicate the status of each agent's action.
enum class ActionStatusType {
  kUnresolved,
  kSuccess,
  kFail,
};

// Direction each agent can be facing.
enum OrientationType {
  kNorth = 0,
  kEast = 1,
  kSouth = 2,
  kWest = 3,
  kInvalid = 4
};

// Different actions used by the agent.
enum class ActionType { kTurnLeft, kTurnRight, kMoveForward, kStay };

class CoopBoxPushingState : public SimMoveState {
 public:
  CoopBoxPushingState(int horizon);

  std::string ActionToString(Player player, Action action) const override;
  std::string ToString() const override;
  bool IsTerminal() const override;
  std::vector<double> Returns() const override;
  std::vector<double> Rewards() const override;
  std::string InformationState(Player player) const {
    SPIEL_CHECK_GE(player, 0);
    SPIEL_CHECK_LT(player, num_players_);
    return absl::StrCat("Observing player: ", player, "\n", ToString());
  }
  void InformationStateAsNormalizedVector(Player player,
                                          std::vector<double>* values) const;
  Player CurrentPlayer() const override {
    return IsTerminal() ? kTerminalPlayerId : cur_player_;
  }
  std::unique_ptr<State> Clone() const override;

  ActionsAndProbs ChanceOutcomes() const;

  void Reset(const GameParameters& params);
  std::vector<Action> LegalActions(Player player) const override;

 protected:
  void DoApplyAction(Action action) override;
  void DoApplyActions(const std::vector<Action>& actions) override;

 private:
  void SetField(std::pair<int, int> coord, char v);
  void SetPlayer(std::pair<int, int> coord, Player player,
                 OrientationType orientation);
  void SetPlayer(std::pair<int, int> coord, Player player);
  void AddReward(double reward);
  char field(std::pair<int, int> coord) const;
  void ResolveMoves();
  void MoveForward(Player player);
  bool InBounds(std::pair<int, int> coord) const;
  bool SameAsPlayer(std::pair<int, int> coord, Player player) const;
  int ObservationPlane(std::pair<int, int> coord, Player player) const;

  // Fields sets to bad/invalid values. Use Game::NewInitialState().
  double total_rewards_ = -1;
  int horizon_ = -1;        // Limit on the total number of moves.
  Player cur_player_ = -1;  // Could be chance's turn.
  int total_moves_ = 0;
  int initiative_;  // 0 = player initiative+1 goes first.
  bool win_;        // True if agents push the big box to the goal.

  // Most recent rewards.
  double reward_;
  // All coordinates below are (row, col).
  std::array<std::pair<int, int>, 2> player_coords_;  // Players' coordinates.
  // Players' orientations.
  std::array<OrientationType, 2> player_orient_;
  // Moves chosen by agents.
  std::array<ActionType, 2> moves_;
  // The status of each of the players' moves.
  std::array<ActionStatusType, 2> action_status_;
  // Actual field used by the players.
  std::vector<char> field_;
};

class CoopBoxPushingGame : public SimMoveGame {
 public:
  explicit CoopBoxPushingGame(const GameParameters& params);
  int NumDistinctActions() const override;
  std::unique_ptr<State> NewInitialState() const override;
  int MaxChanceOutcomes() const override { return 4; }
  int NumPlayers() const override;
  double MinUtility() const override;
  double MaxUtility() const override;
  std::unique_ptr<Game> Clone() const override {
    return std::unique_ptr<Game>(new CoopBoxPushingGame(*this));
  }
  std::vector<int> InformationStateNormalizedVectorShape() const override;
  int MaxGameLength() const override { return horizon_; }

 private:
  int horizon_;
};

}  // namespace coop_box_pushing
}  // namespace open_spiel

#endif  // THIRD_PARTY_OPEN_SPIEL_GAMES_COOP_BOX_PUSHING
