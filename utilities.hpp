#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <string_view>
#include <sstream>
#include <chrono>

inline std::string read_all_from_file(std::string_view filepath)
{
	std::ifstream file_stream(filepath.data());

	if (file_stream.is_open() == false) {
		std::cerr << "[FILE] cannot open " << filepath << std::endl;
		std::terminate();
	}

	std::stringstream buffer;
	buffer << file_stream.rdbuf();

	return buffer.str();
}

class Timer {
public:
	void start() {
		if (running == true) return;
		running = true;
		begin = std::chrono::high_resolution_clock::now();
		current = begin;
	}

	float delta_time() {
		auto now = std::chrono::high_resolution_clock::now();
		auto dt = std::chrono::duration<float>(now - current);
		current = now;
		return dt.count();
	}

	float end() {
		if (running == false) return -1.f;
		running = false;
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed_time = std::chrono::duration<float>(end - begin);

		return elapsed_time.count();
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> begin;
	std::chrono::time_point<std::chrono::high_resolution_clock> current;

	bool running = false;
};