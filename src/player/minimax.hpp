#pragma once

#include "board.hpp"
#include "player/base.hpp"
#include "player/minimax/evaluator.hpp"
#include "player/minimax/zobrist.hpp"

#include <functional>
#include <vector>
#include <random>
#include <unordered_map>
#include <fmt/core.h>

namespace tkz::gomoku::minimax {

template <::std::invocable C, ::std::invocable D>
struct L {
	[[no_unique_address]] C c;
	[[no_unique_address]] D d;

	L(C c, D d): c(::std::move(c)), d(::std::move(d)) { ::std::invoke(this->c); }
	~L() { ::std::invoke(this->d); }
};

struct MinimaxPlayer : public Player {
	Evaluator eval;
	Zobrist zobrist{::std::mt19937_64{::std::random_device{}()}};
	int depth = 4;
	int radius = 2;

	::std::unordered_map<::std::uint_fast64_t, Position> best{{0, {7, 7}}};

	::std::unordered_map<::std::uint_fast64_t, Position>::iterator updateBest(::std::unordered_map<::std::uint_fast64_t, Position>::iterator it, ::std::uint_fast64_t hash, Position pos) {
		if (it != best.end()) {
			it->second = pos;
			return it;
		}
		return best.emplace(hash, pos).first;
	}

	::std::unordered_map<::std::uint_fast64_t, double> cache;

	struct Searcher {
		MinimaxPlayer* player;
		Board board;
		::std::vector<Step> steps;
		::std::uint_fast64_t hash;

		auto doStep(Side side, Position pos) {
			return L(
				[=, this] {
					board[pos] = ::std::visit(
						overloaded{
							make_matcher<Black>(Cell{Black{}}),
							make_matcher<White>(Cell{White{}}),
						},
						side
					);
					steps.push_back({ .side = side, .pos = pos });
					hash ^= player->zobrist(side, pos);
				},
				[=, this] {
					board[pos] = ::std::monostate{};
					steps.pop_back();
					hash ^= player->zobrist(side, pos);
				}
			);
		}

		double min(int depth, double alpha, double beta) {
			Side side = steps.empty() ? Black{} : alter(steps.back().side);
			if (depth == player->depth || !steps.empty() && board.isWinningPos(steps.back().pos)) {
				double ally = player->eval(board, side);
				double enemy = player->eval(board, alter(side));
				double score = ally - enemy;
				// ::fmt::println("{:016x} (depth={}) : {} = {} - {}", hash, depth, score, ally, enemy);
				return score;
			}
			auto callMax = [&](Position pos) {
				auto _ = doStep(side, pos);
				return max(depth + 1, alpha, beta);
			};
			auto it = player->best.find(hash);
			if (it != player->best.end() && ::std::holds_alternative<::std::monostate>(board[it->second])) {
				// ::fmt::println("best first");
				double score = callMax(it->second);
				if (score < beta) {
					beta = score;
				}
				if (alpha >= beta) {
					// ::fmt::println("{:016x} (depth={}) pruning", hash, depth);
					return beta;
				}
			}
			for (auto&& step : steps) {
				for (int k = 1; k <= player->radius; ++k) {
					for (auto&& d : directions) {
						auto q = step.pos + k * d;
						if (Position::valid(q) && ::std::holds_alternative<::std::monostate>(board[q])) {
							double score = callMax(q);
							if (score < beta) {
								beta = score;
								it = player->updateBest(it, hash, q);
							}
							if (alpha >= beta) {
								// ::fmt::println("{:016x} (depth={}) pruning", hash, depth);
								return beta;
							}
						}
					}
				}
			}
			return beta;
		}

		double max(int depth, double alpha, double beta) {
			Side side = steps.empty() ? Black{} : alter(steps.back().side);
			if (depth == player->depth || !steps.empty() && board.isWinningPos(steps.back().pos)) {
				double ally = player->eval(board, side);
				double enemy = player->eval(board, alter(side));
				double score = ally - enemy;
				// ::fmt::println("{:016x} (depth={}) : {} = {} - {}", hash, depth, score, ally, enemy);
				return score;
			}
			auto callMin = [&](Position pos) {
				auto _ = doStep(side, pos);
				return min(depth + 1, alpha, beta);
			};
			auto it = player->best.find(hash);
			if (it != player->best.end() && ::std::holds_alternative<::std::monostate>(board[it->second])) {
				// ::fmt::println("{:016x} (depth={}) best", hash, depth);
				double score = callMin(it->second);
				if (score > alpha) {
					alpha = score;
				}
				if (alpha >= beta) {
					// ::fmt::println("{:016x} (depth={}) pruning", hash, depth);
					return alpha;
				}
			}
			for (auto&& step : steps) {
				for (int k = 1; k <= player->radius; ++k) {
					for (auto&& d : directions) {
						auto q = step.pos + k * d;
						if (Position::valid(q) && ::std::holds_alternative<::std::monostate>(board[q])) {
							double score = callMin(q);
							if (score > alpha) {
								alpha = score;
								it = player->updateBest(it, hash, q);
							}
							if (alpha >= beta) {
								// ::fmt::println("{:016x} (depth={}) pruning", hash, depth);
								return alpha;
							}
						}
					}
				}
			}
			return alpha;
		}
	};

	Operation decide(::std::span<Step const> steps) override {
		auto hash = this->zobrist(steps);
		Searcher{
			.player = this,
			.board = Board::fromSteps(steps),
			.steps{steps.begin(), steps.end()},
			.hash = hash,
		}.max(
			0,
			-::std::numeric_limits<double>::infinity(),
			+::std::numeric_limits<double>::infinity()
		);
		return best.find(hash)->second;
	}
};

}

namespace tkz::gomoku {

using MinimaxPlayer = minimax::MinimaxPlayer;

}
