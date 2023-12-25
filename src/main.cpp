#include "board.hpp"
#include "player/base.hpp"
#include "player/minimax.hpp"
#include "ui.hpp"

#include <memory>
#include <vector>
#include <cstdlib>

int main() {
	using namespace ::std;
	using namespace ::tkz::gomoku;

	unique_ptr<Player> blackPlayer = make_unique<UIPlayer>();
	unique_ptr<Player> whitePlayer = make_unique<AsyncPlayer>(make_unique<MinimaxPlayer>());
	vector<Step> steps;

	InitWindow(width, height, "Gomoku");
	while (!WindowShouldClose()) {
		BeginDrawing();
		drawBoard();
		drawSteps(steps);
		EndDrawing();
		Side side = steps.empty() ? Black{} : alter(steps.back().side);
		Operation op = visit(
			overloaded{
				make_matcher<Black>(ref(blackPlayer)),
				make_matcher<White>(ref(whitePlayer)),
			},
			side
		).get()->decide(steps);
		visit(
			[&]<typename Op>(Op op) {
				if constexpr (is_same_v<Op, Position>) {
					steps.push_back({ .side = side, .pos = op });
					if (Board::fromSteps(steps).isWinningPos(op)) {
						BeginDrawing();
						drawBoard();
						drawSteps(steps);
						EndDrawing();
						WaitTime(5);
						exit(EXIT_SUCCESS);
					}
				}
				if constexpr (is_same_v<Op, Retract>) {
					if (steps.size() >= 2) {
						steps.pop_back();
						steps.pop_back();
					}
				}
				if constexpr (is_same_v<Op, GiveUp>) {
					WaitTime(5);
					exit(EXIT_SUCCESS);
				}
			},
			op
		);
	}
}