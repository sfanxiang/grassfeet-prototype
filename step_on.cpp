#include <cstdint>
#include <cstdio>
#include <functional>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>

struct Transform {
	float x, y, z;

	Transform(): x(), y(), z() {}
};

enum class PointFillStatus {
	Empty,
	Path,
	Grass
};

// 1. Because we use graph to represent data, we have to ensure the player
//    never go from one point to a non-neighboring point. And grass can only be
//    filled by neighboring points.
struct Point {
	PointFillStatus fill_status;
	bool has_cow;
	Transform transform;
	std::vector<uint32_t> next;

	Point(): fill_status(PointFillStatus::Empty), has_cow() {}
};

struct FloodFillResult {
	uint32_t filled;
	bool has_visited;
	bool has_cow;
	bool path_only;

	FloodFillResult(): filled(), has_visited(), has_cow(), path_only(true) {}
};

// Result is only valid when called without set_grass.
static FloodFillResult flood_fill(
	std::vector<Point> &points,
	uint32_t index,
	std::vector<bool> &visited,
	bool set_grass,
	std::function<void(uint32_t, PointFillStatus)> &fill
) {
	FloodFillResult result;

	if (visited[index]) {
		result.has_visited = true;
		return result;
	}
	if (points[index].fill_status != PointFillStatus::Empty) {
		return result;
	}
	if (points[index].has_cow) {
		result.has_cow = true;
		return result;
	}
	result.filled++;
	fill(index, PointFillStatus::Grass);
	if (set_grass) {
		points[index].fill_status = PointFillStatus::Grass;
	}

	std::vector<bool> used(points.size());
	used[index] = visited[index] = true;

	std::queue<uint32_t> fill_queue;
	fill_queue.push(index);

	while (!fill_queue.empty()) {
		index = fill_queue.front();
		fill_queue.pop();

		for (auto &i: points[index].next) {
			if (!used[i]) {
				switch (points[i].fill_status) {
				case PointFillStatus::Empty:
					if (visited[i]) {
						result.has_visited = true;
						return result;
					}
					if (points[i].has_cow) {
						result.has_cow = true;
						return result;
					}
					result.filled++;
					fill(index, PointFillStatus::Grass);
					if (set_grass) {
						points[i].fill_status = PointFillStatus::Grass;
					}
					used[i] = visited[i] = true;
					fill_queue.push(i);
					break;
				case PointFillStatus::Path:
					if (points[i].has_cow) {
						result.has_cow = true;
						return result;
					}
					fill(index, PointFillStatus::Grass);
					if (set_grass) {
						points[i].fill_status = PointFillStatus::Grass;
					}
					break;
				case PointFillStatus::Grass:
					if (points[i].has_cow) {
						result.has_cow = true;
						return result;
					}
					result.path_only = false;
					break;
				}
			}
		}
	}

	return result;
}

void step_on(
	std::vector<Point> &points, uint32_t index, uint32_t max_fill,
	bool set_status = true,
	std::function<void(uint32_t, PointFillStatus)> fill
		= [](uint32_t, PointFillStatus) {}
) {
	Point &point = points[index];

	auto prev_fill_status = point.fill_status;

	// Mark the current point.
	fill(index, PointFillStatus::Path);
	point.fill_status = PointFillStatus::Path;

	// Flood fill.

	enum class Status {
		None, Single, Multiple
	};
	Status status = Status::None;
	uint32_t single_index = 0;
	bool single_path_only = false;

	std::vector<bool> visited(points.size());

	for (auto &i: point.next) {
		std::function<void(uint32_t, PointFillStatus)> fill_nop =
			[](uint32_t, PointFillStatus) {};
		auto result = flood_fill(points, i, visited, false, fill_nop);

		if (result.filled > 0 && result.filled <= max_fill &&
			!result.has_cow && !result.has_visited)
		{
			std::vector<bool> tmp_visited(points.size());

			switch (status) {
			case Status::None:
				status = Status::Single;
				single_index = i;
				single_path_only = result.path_only;
				break;
			case Status::Single:
				status = Status::Multiple;
				flood_fill(points, single_index, tmp_visited, set_status, fill);
				flood_fill(points, i, tmp_visited, set_status, fill);
				break;
			case Status::Multiple:
				flood_fill(points, i, tmp_visited, set_status, fill);
				break;
			}
		}
	}
	if (status == Status::Single && single_path_only) {
		std::vector<bool> tmp_visited(points.size());
		flood_fill(points, single_index, tmp_visited, set_status, fill);
	}

	if (!set_status)
		point.fill_status = prev_fill_status;
}

static void print_status(std::vector<Point> &points)
{
	std::cout << "Path:" << std::endl;
	for (uint32_t i = 0; i < points.size(); i++) {
		if (points[i].fill_status == PointFillStatus::Path) {
			std::cout << i << " ";
		}
	}
	std::cout << std::endl;

	std::cout << "Grass:" << std::endl;
	for (uint32_t i = 0; i < points.size(); i++) {
		if (points[i].fill_status == PointFillStatus::Grass) {
			std::cout << i << " ";
		}
	}
	std::cout << std::endl << std::endl;
}

int main()
{
	uint32_t n, max_fill;

	// Map
	std::cin >> n >> max_fill;
	std::vector<Point> points(n);

	for (uint32_t i = 0; i < n; i++) {
		uint32_t neighbors;
		std::cin >> neighbors;

		points[i].next.resize(neighbors);

		for (uint32_t j = 0; j < neighbors; j++) {
			uint32_t neighbor;
			std::cin >> neighbor;
			if (neighbor < points.size()) {
				points[i].next[j] = neighbor;
			}
		}
	}

	// Cows
	std::cin >> n;
	for (uint32_t i = 0; i < n; i++) {
		uint32_t cow;
		std::cin >> cow;
		if (cow < points.size()) {
			points[cow].has_cow = true;
		}
	}

	// Steps
	while (std::cin >> n) {
		if (n < points.size()) {
			step_on(points, n, max_fill);
			print_status(points);
		}
	}
}
