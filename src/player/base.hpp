#pragma once

#include "board.hpp"

#include <variant>
#include <span>

namespace tkz::gomoku {

struct Retract {};
struct GiveUp {};

using Operation = ::std::variant<Position, Retract, GiveUp>;

struct Player {
	virtual ~Player() = default;
	virtual Operation decide(::std::span<Step const> steps) = 0;
};

}
