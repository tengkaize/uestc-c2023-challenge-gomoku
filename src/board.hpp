#pragma once

#include <variant>
#include <array>
#include <span>

namespace tkz::gomoku {

inline constexpr int rows = 15;
inline constexpr int cols = 15;

struct Black {
	friend constexpr bool operator==(Black, Black) = default;
};

struct White {
	friend constexpr bool operator==(White, White) = default;
};

using Side = ::std::variant<Black, White>;

template <typename... Fs>
struct overloaded : Fs... {
	using Fs::operator()...;
};

template <typename... Fs>
overloaded(Fs...) -> overloaded<Fs...>;

template <typename T, typename V>
constexpr auto make_matcher(V&& v) { return [v = ::std::forward<V>(v)](T){ return v; }; }

inline Side alter(Side side) {
	return ::std::visit(overloaded{
		make_matcher<Black>(Side{White{}}),
		make_matcher<White>(Side{Black{}}),
	}, side);
}

using Cell = ::std::variant<::std::monostate, Black, White>;

struct Position {
	int x, y;

	friend constexpr bool operator==(Position, Position) = default;

	static constexpr Position fromIndex(int i) {
		return {
			.x = i / cols,
			.y = i % cols
		};
	}

	static constexpr int toIndex(Position pos) {
		return pos.x * cols + pos.y;
	}

	static constexpr bool valid(Position pos) {
		return (0 <= pos.x && pos.x < rows) && (0 <= pos.y && pos.y < cols);
	}
};

struct Difference {
	int x, y;

	friend constexpr Position operator+(Position pos, Difference diff) {
		return {
			.x = pos.x + diff.x,
			.y = pos.y + diff.y
		};
	}

	friend constexpr Difference operator*(int k, Difference d) {
		return {
			.x = k * d.x,
			.y = k * d.y
		};
	}
};

inline constexpr Difference directions[8] = {
	{1, 0}, // 0: right
	{1, 1}, // 1: rightUp
	{0, 1}, // 2: up
	{-1, 1}, // 3: leftUp
	{-1, 0}, // 4: left
	{-1, -1}, // 5: leftDown
	{0, -1}, // 6: down
	{1, -1}, // 7: rightDown
};

struct Step {
	Side side;
	Position pos;
};

struct Board {
	::std::array<Cell, rows * cols> cells;

	Cell& operator[](Position pos) { return this->cells[Position::toIndex(pos)]; }
	Cell const& operator[](Position pos) const { return this->cells[Position::toIndex(pos)]; }

	bool isWinningPos(Position pos) const {
		if (::std::holds_alternative<::std::monostate>((*this)[pos])) {
			return false;
		}
		::std::array<int, 8> count{};
		for (int i = 0; i < 8; ++i) {
			for (auto q = pos + directions[i]; Position::valid(q); q = q + directions[i]) {
				if ((*this)[pos] != (*this)[q]) {
					break;
				}
				++count[i];
			}
		}
		for (int i = 0; i < 4; ++i) {
			if (count[i] + 1 + count[i ^ 4] >= 5) {
				return true;
			}
		}
		return false;
	}

	static Board fromSteps(::std::span<Step const> steps) {
		Board board;
		for (auto&& [side, pos] : steps) {
			board[pos] = ::std::visit(
				overloaded{
					make_matcher<Black>(Cell{Black{}}),
					make_matcher<White>(Cell{White{}})
				},
				side
			);
		}
		return board;
	}
};

}
