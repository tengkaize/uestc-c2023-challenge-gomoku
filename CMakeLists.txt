cmake_minimum_required(VERSION 3.26)

project(gomoku)

add_executable(gomoku)

target_compile_features(gomoku
	PRIVATE
		cxx_std_23
)

target_include_directories(gomoku PRIVATE src)

target_sources(gomoku
	PRIVATE
		src/main.cpp
		src/board.hpp
		src/player/base.hpp
		src/player/minimax.hpp
		src/player/minimax/evaluator.hpp
		src/player/minimax/zobrist.hpp
		src/player/mcts.hpp
		src/ui.hpp
)

find_package(raylib REQUIRED)
find_package(Boost REQUIRED COMPONENTS container)
find_package(fmt REQUIRED)
target_link_libraries(gomoku
	PRIVATE
		raylib
		Boost::container
		fmt::fmt
)
