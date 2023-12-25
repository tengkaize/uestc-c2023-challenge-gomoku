#pragma once

#include "board.hpp"
#include "player/base.hpp"

#include <cmath>
#include <algorithm>
#include <ranges>
#include <memory>
#include <future>
#include <chrono>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>
#include <raylib.h>

namespace tkz::gomoku {

inline constexpr int width = 600;
inline constexpr int height = 600;
inline constexpr int cellWidth = width / rows;
inline constexpr int cellHeight = height / cols;
inline constexpr int boardWidth = (rows - 1) * cellWidth;
inline constexpr int boardHeight = (cols - 1) * cellHeight;
inline constexpr int paddingLeft = (width - boardWidth) / 2;
inline constexpr int paddingTop = (height - boardHeight) / 2;
inline constexpr int pieceRadius = 15;
inline constexpr int thick = 3;

inline constexpr int xi2p(int i) { return paddingLeft + cellWidth * i; }
inline constexpr int yi2p(int i) { return paddingTop + cellHeight * i; }
inline constexpr int xp2i(float p) {
	return ::std::clamp<int>(::std::round((p - paddingLeft) / cellWidth), 0, rows - 1);
}
inline constexpr int yp2i(float p) {
	return ::std::clamp<int>(::std::round((p - paddingTop) / cellHeight), 0, cols - 1);
}

inline Color colorOf(Side side) {
	return ::std::visit(
		overloaded{
			make_matcher<Black>(BLACK),
			make_matcher<White>(WHITE),
		},
		side
	);
}

inline void drawPiece(Position pos, Side side) {
	DrawCircle(
		xi2p(pos.x), yi2p(pos.y),
		pieceRadius,
		colorOf(side)
	);
}

inline void drawGradient(Position pos, Color inner, Color outer) {
	DrawCircleGradient(
		xi2p(pos.x), yi2p(pos.y),
		pieceRadius,
		inner,
		outer
	);
}

inline void drawBoard() {
	using namespace std;
	ClearBackground(BEIGE);
	DrawCircle(xi2p(3), yi2p(3), 6, DARKBROWN);
	DrawCircle(xi2p(3), yi2p(11), 6, DARKBROWN);
	DrawCircle(xi2p(11), yi2p(3), 6, DARKBROWN);
	DrawCircle(xi2p(11), yi2p(11), 6, DARKBROWN);
	DrawCircle(xi2p(7), yi2p(7), 6, DARKBROWN);
	ranges::for_each(views::iota(0, rows) | views::transform(xi2p), [](float x) {
		DrawLineEx(
			{x, paddingTop},
			{x, paddingTop + boardHeight},
			thick,
			DARKBROWN
		);
	});
	ranges::for_each(views::iota(0, cols) | views::transform(yi2p), [](float y) {
		DrawLineEx(
			{paddingLeft, y},
			{paddingLeft + boardWidth, y},
			thick,
			DARKBROWN
		);
	});
}

inline void drawSteps(::std::span<Step const> steps) {
	if (steps.empty()) return;
	for (auto&& [side, pos] : steps) {
		drawPiece(pos, side);
	}
	drawGradient(steps.back().pos, GOLD, BLANK);
}

struct UIPlayer : public Player {
	Operation decide(::std::span<Step const> steps) override {
		Side side = steps.empty() ? Black{} : alter(steps.back().side);
		auto board = Board::fromSteps(steps);
		while (!WindowShouldClose()) {
			BeginDrawing();
			drawBoard();
			drawSteps(steps);
			auto [xp, yp] = GetMousePosition();
			Position pos{xp2i(xp), yp2i(yp)};
			if (::std::holds_alternative<::std::monostate>(board[pos])) {
				drawGradient(pos, BLANK, colorOf(side));
			}
			else {
				drawGradient(pos, BLANK, RED);
			}
			EndDrawing();
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && ::std::holds_alternative<::std::monostate>(board[pos])) {
				return pos;
			}
			if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				return Retract{};
			}
			if (IsKeyPressed(KEY_Q)) {
				return GiveUp{};
			}
		}
		return GiveUp{};
	}
};

struct AsyncPlayer : public Player {
	::std::unique_ptr<Player> underlying;
	AsyncPlayer(::std::unique_ptr<Player> underlying): underlying(::std::move(underlying)) {}
	Operation decide(::std::span<Step const> steps) override {
		using namespace ::std;
		using namespace ::std::chrono;
		future<Operation> future = async(launch::async, [&] {
			return this->underlying->decide(steps);
		});
		auto start = steady_clock::now();
		do {
			BeginDrawing();
			drawBoard();
			drawSteps(steps);
			EndDrawing();
		} while (future.wait_for(0ms) != future_status::ready);
		auto end = steady_clock::now();
		{
			using namespace ::fmt;
			println("player take {} to think the next step.",
				styled(
					duration_cast<milliseconds>(end - start),
					fg(color::yellow) | emphasis::bold
				)
			);
		}
		return future.get();
	}
};

}
