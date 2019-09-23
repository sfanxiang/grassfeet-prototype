#include <cstdint>
#include <cstdio>
#include <iostream>
#include <queue>
#include <utility>
#include <vector>

struct Transform {
	float x, y, z;

	Transform(): x(), y(), z() {}
};

// 1. Here "path" and "grass" behave differently. One consequence of this is
//    that we have to highlight the path for the player.
// 2. Because we use graph to represent data, we have to ensure the player
//    never go from one point to a non-neighboring point. And grass can only be
//    filled by neighboring points.
// 3. This can be improved so that each player has their own path.
struct Point {
	bool in_path, has_grass, has_cow;
	Transform transform;
	std::vector<uint32_t> next;

	Point(): in_path(), has_grass(), has_cow() {}
};

struct FloodFillResult {
	uint32_t filled;
	bool has_cow;

	FloodFillResult(): filled(), has_cow() {}
};

FloodFillResult flood_fill(
	std::vector<Point> &points,
	uint32_t index,
	uint32_t max_fill,
	bool set_grass
) {
	FloodFillResult result;

	if (points[index].in_path) return result;

	result.filled++;

	std::vector<bool> visited;
	visited.resize(points.size());
	visited[index] = true;

	std::queue<uint32_t> fill_queue;
	fill_queue.push(index);

	result.has_cow = result.has_cow || points[index].has_cow;
	if (set_grass) {
		points[index].has_grass = true;
	}

	if (max_fill < result.filled) return result;

	while (!fill_queue.empty()) {
		index = fill_queue.front();
		fill_queue.pop();

		for (auto &i: points[index].next) {
			if (!visited[i] && !points[i].in_path) {
				result.filled++;
				visited[i] = true;
				fill_queue.push(i);

				result.has_cow = result.has_cow || points[i].has_cow;
				if (set_grass) {
					points[i].has_grass = true;
				}

				if (max_fill < result.filled) return result;
			}
		}
	}

	return result;
}

// I found it hard (if possible at all) to define the exterior of a shape on a
// closed surface. I used max_fill as a workaround, but we shouldn't rely on
// it (the player can just fill out the area manually until it reaches below
// max_fill). I'd recommend restricting cows to certain areas, distributed
// across the map, to prevent filling out the entire planet.
void step_on(std::vector<Point> &points, uint32_t index, uint32_t max_fill)
{
	Point &point = points[index];

	// Mark the current point.
	point.in_path = point.has_grass = true;

	// Flood fill.
	bool has_valid_enclosure = false;
	for (auto &i: point.next) {
		auto result = flood_fill(points, i, max_fill, false);
		if (result.filled > 0 && result.filled <= max_fill) {
			has_valid_enclosure = true;
			if (!result.has_cow) {
				flood_fill(points, i, max_fill, true);
			}
		}
	}

	if (has_valid_enclosure) {
		// Delete all paths.
		for (auto &point: points) {
			point.in_path = false;
		}
	}
}

static void print_status(std::vector<Point> &points)
{
	std::cout << "Path:" << std::endl;
	for (int i = 0; i < points.size(); i++) {
		if (points[i].in_path) {
			std::cout << i << " ";
		}
	}
	std::cout << std::endl;

	std::cout << "Grass:" << std::endl;
	for (int i = 0; i < points.size(); i++) {
		if (points[i].has_grass) {
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
	std::vector<Point> points;
	points.resize(n);

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
