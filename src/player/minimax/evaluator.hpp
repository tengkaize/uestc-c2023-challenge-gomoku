#pragma once

#include "board.hpp"

#include <utility>
#include <algorithm>
#include <string_view>
#include <vector>
#include <boost/container/static_vector.hpp>

namespace tkz::gomoku::minimax {

struct Evaluator {
	double 成五 = 50000000.0;
	double 活四 = 1000000.0;
	double 冲四 = 100000.0;
	double 单活三 = 80000.0;
	double 条活三 = 70000.0;
	double 眠三 = 5000.0;
	double 活二 = 500.0;
	double 眠二 = 100.0;

	static inline const auto mappings = []{
		using namespace ::std;
		using Rule = string_view;
		using RuleList = vector<Rule>;
		using Mapping = pair<double Evaluator::*, RuleList>;
		using MappingList = vector<Mapping>;

		return MappingList{
			Mapping{
				&Evaluator::成五,
				RuleList{
					"sssss"sv,
				},
			},
			Mapping{
				&Evaluator::活四,
				RuleList{
					"esssse"sv,
				},
			},
			Mapping{
				&Evaluator::冲四,
				RuleList{
					"esssst"sv,
					"tsssse"sv,
					"sesss"sv,
					"ssses"sv,
					"ssess"sv,
				},
			},
			Mapping{
				&Evaluator::单活三,
				RuleList{
					"essse"sv,
				},
			},
			Mapping{
				&Evaluator::条活三,
				RuleList{
					"sess"sv,
					"sses"sv,
				},
			},
			Mapping{
				&Evaluator::眠三,
				RuleList{
					"eessst"sv,
					"tsssee"sv,
					"esesst"sv,
					"tssese"sv,
					"essest"sv,
					"tsesse"sv,
					"seess"sv,
					"ssees"sv,
					"seses"sv,
					"tessset"sv,
				},
			},
			Mapping{
				&Evaluator::活二,
				RuleList{
					"eessee"sv,
					"esese"sv,
					"sees"sv,
				},
			},
			Mapping{
				&Evaluator::眠二,
				RuleList{
					"eeesst"sv,
					"tsseee"sv,
					"eesest"sv,
					"tsesee"sv,
					"eseest"sv,
					"tseese"sv,
					"seees"sv,
				},
			},
		};
	}();

	static inline int posScore(Position pos) {
		return ::std::min(
			::std::min(
				pos.x,
				rows - 1 - pos.x
			),
			::std::min(
				pos.y,
				cols - 1 - pos.y
			)
		);
	};

	double operator()(Board const& board, Side side) const {
		using namespace ::std;
		using ::boost::container::static_vector;

		auto toChar = overloaded{
			make_matcher<Black>(holds_alternative<Black>(side) ? 's' : 't'),
			make_matcher<White>(holds_alternative<White>(side) ? 's' : 't'),
			make_matcher<monostate>('e')
		};
		auto evalPos = [&](Position pos){
			array<static_vector<char, 9>, 4> lines;
			for (int i = 0; i < 4; ++i) {
				for (int k = -4; k <= 4; ++k) {
					Position q = pos + k * directions[i];
					if (!Position::valid(q)) continue;
					lines[i].push_back(visit(toChar, board[q]));
				}
			}
			double total = posScore(pos);
			for (auto&& line : lines) {
				auto contains = [line = string_view{line}](string_view rule) { return line.contains(rule); };
				for (auto&& [member, rules] : mappings) {
					if (ranges::any_of(rules, contains)) {
						total += this->*member;
					}
				}
			}
			return total;
		};
		return visit(
			[&]<typename T>(T) {
				double score = 0.0;
				for (int x = 0; x < rows; ++x) {
					for (int y = 0; y < cols; ++y) {
						if (holds_alternative<T>(board[{x, y}])) {
							score += evalPos({x, y});
						}
					}
				}
				return score;
			},
			side
		);
	}
};

}
