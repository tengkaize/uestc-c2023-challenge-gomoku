#pragma once

#include "board.hpp"

#include <random>

namespace tkz::gomoku::minimax {

struct Zobrist {
	::std::array<::std::uint_fast64_t, rows * cols> black{};
	::std::array<::std::uint_fast64_t, rows * cols> white{};

	template <::std::uniform_random_bit_generator G>
	explicit Zobrist(G gen) {
		for (int i = 0; i < rows * cols; ++i) this->black[i] = gen();
		for (int i = 0; i < rows * cols; ++i) this->white[i] = gen();
	}

	::std::uint_fast64_t operator()(Side side, Position pos) const {
		return ::std::visit(
			overloaded{
				make_matcher<Black>(::std::ref(this->black)),
				make_matcher<White>(::std::ref(this->white)),
			},
			side
		).get()[Position::toIndex(pos)];
	}

	::std::uint_fast64_t operator()(::std::span<Step const> steps) const {
		::std::uint_fast64_t value{};
		for (auto&& [side, pos] : steps) {
			value ^= (*this)(side, pos);
		}
		return value;
	}
};

}