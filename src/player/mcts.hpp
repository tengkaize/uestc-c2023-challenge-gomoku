#pragma once

#include "board.hpp"
#include "player/base.hpp"
#include "player/minimax/evaluator.hpp"

#include <memory>
#include <cmath>
#include <numbers>
#include <vector>
#include <unordered_map>
#include <variant>
#include <algorithm>
#include <random>
#include <optional>
#include <fmt/core.h>

namespace tkz::gomoku {

struct MCTSPlayer : public Player {
	static ::std::optional<Side> getWinner(Board const& board) {
		using namespace ::std;
		for (int i = 0; i < rows * cols; ++i) {
			auto pos = Position::fromIndex(i);
			if (board.isWinningPos(pos)) {
				return visit(
					[]<typename T>(T) -> optional<Side> {
						if constexpr (is_same_v<T, Black>) return Black{};
						if constexpr (is_same_v<T, White>) return White{};
						return nullopt;
					},
					board[pos]
				);
			}
		}
		return nullopt;
	}

	static ::std::vector<Position> getChoices(Board const& board) {
		::std::vector<Position> choices;
		for (int i = 0; i < rows * cols; ++i) {
			auto pos = Position::fromIndex(i);
			if (::std::holds_alternative<::std::monostate>(board[pos])) {
				choices.push_back(pos);
			}
		}
		return choices;
	}

	struct Node : public ::std::enable_shared_from_this<Node> {
		::std::weak_ptr<Node> parent;
		::std::unordered_map<int, ::std::shared_ptr<Node>> children;
		int visitTimes;
		double quality;
		Side const side;
		Board const board;
		bool const terminal;
		::std::vector<Position> choices;

		Node(Side side, Board const& board):
			parent(),
			children(),
			visitTimes(0),
			quality(0.0),
			side(side),
			board(board),
			terminal(getWinner(board).has_value()),
			choices(getChoices(board))
		{ }

		Node(::std::shared_ptr<Node> parent, Board const& board):
			parent(parent),
			children(),
			visitTimes(0),
			quality(0.0),
			side(alter(parent->side)),
			board(board),
			terminal(getWinner(board).has_value()),
			choices(getChoices(board))
		{ }

		::std::shared_ptr<Node> expand() {
			static ::std::mt19937_64 gen{};
			::std::shuffle(choices.begin(), choices.end(), gen);
			Position pos = choices.back();
			choices.pop_back();
			Board nextBoard = this->board;
			nextBoard[pos] = ::std::visit(
				overloaded{
					make_matcher<Black>(Cell{Black{}}),
					make_matcher<White>(Cell{White{}}),
				},
				this->side
			);
			return children.emplace(
				Position::toIndex(pos),
				::std::make_shared<Node>(
					shared_from_this(),
					nextBoard
				)
			).first->second;
		}

		::std::shared_ptr<Node> bestUCT() const {
			double bestScore = -::std::numeric_limits<double>::infinity();
			::std::shared_ptr<Node> bestChild = nullptr;
			for (auto&& [_, child] : this->children) {
				double left = child->quality / child->visitTimes;
				double right = 2.0 * ::std::log(this->visitTimes) / child->visitTimes;
				double score = left + 1.0 / ::std::numbers::sqrt2 * right;
				if (score > bestScore) {
					bestScore = score;
					bestChild = child;
				}
			}
			return bestChild;
		}

		int bestIndex() const {
			double bestScore = -::std::numeric_limits<double>::infinity();
			int bestIndex = -1;
			for (auto&& [index, child] : this->children) {
				double score = child->quality / child->visitTimes;
				if (score > bestScore) {
					bestScore = score;
					bestIndex = index;
				}
			}
			return bestIndex;
		}

		void backPropagate(double reward) {
			this->visitTimes += 1;
			this->quality += reward;
			if (auto parent = this->parent.lock()) {
				parent->backPropagate(-reward);
			}
		}
	};

	int times = 100000;
	minimax::Evaluator eval;

	static ::std::shared_ptr<Node> select(::std::shared_ptr<Node> node) {
		while (!node->terminal) {
			if (node->choices.empty()) {
				node = node->bestUCT();
			}
			else {
				return node->expand();
			}
		}
		return node;
	}

	double simulate(::std::shared_ptr<Node> node) const {
		double ally = eval(node->board, alter(node->side));
		double enemy = eval(node->board, node->side);
		return ally - enemy;
	}

	Operation decide(::std::span<const Step> steps) override {
		Side side = steps.empty() ? Black{} : alter(steps.back().side);
		::std::shared_ptr<Node> root = ::std::make_shared<Node>(side, Board::fromSteps(steps));
		for (int i = 0; i < times; ++i) {
			auto expandNode = select(root);
			double reward = simulate(expandNode);
			expandNode->backPropagate(reward);
		}
		return Position::fromIndex(root->bestIndex());
	}
};

}
