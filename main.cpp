#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

#define PI 3.141592653589793

float toRadians(float degrees) { return degrees * (PI / 180); }
char getDrawChar(float val) { return val > 0.3 ? '#' : ' '; }
using matrix4x4 = std::array<std::array<float, 4>, 4>;

class vec2 {
  public:
	vec2() : x(0), y(0) {};
	vec2(float x, float y) : x(x), y(y) {};
	float x, y;
};

class vec3 {
  public:
	vec3() : x(0), y(0), z(0) {};
	vec3(float x, float y, float z) : x(x), y(y), z(z) {};
	float x, y, z;
};

class vec4 {
  public:
	vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};
	vec4(vec3 point) : x(point.x), y(point.y), z(point.z), w(1.0f) {};
	float x, y, z, w;
};

matrix4x4 rotateX(float angle) {
	float c = std::cos(toRadians(angle));
	float s = std::sin(toRadians(angle));
	return {{{{1, 0, 0, 0}}, {{0, c, -s, 0}}, {{0, s, c, 0}}, {{0, 0, 0, 1}}}};
}

matrix4x4 rotateY(float angle) {
	float c = std::cos(toRadians(angle));
	float s = std::sin(toRadians(angle));
	return {{{{c, 0, s, 0}}, {{0, 1, 0, 0}}, {{-s, 0, c, 0}}, {{0, 0, 0, 1}}}};
}

matrix4x4 rotateZ(float angle) {
	float c = std::cos(toRadians(angle));
	float s = std::sin(toRadians(angle));
	return {{{{c, -s, 0, 0}}, {{s, c, 0, 0}}, {{0, 0, 1, 0}}, {{0, 0, 0, 1}}}};
}

matrix4x4 multMat(matrix4x4 &a, matrix4x4 &b) {
	matrix4x4 res{};
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			for (int k = 0; k < 4; k++)
				res[i][j] += a[i][k] * b[k][j];
	return res;
}

vec4 multVec(vec4 &point, matrix4x4 &matrix) {
	vec4 res(0, 0, 0, 0);
	res.x = matrix[0][0] * point.x + matrix[1][0] * point.y +
			matrix[2][0] * point.z + matrix[3][0] * point.w;
	res.y = matrix[0][1] * point.x + matrix[1][1] * point.y +
			matrix[2][1] * point.z + matrix[3][1] * point.w;
	res.z = matrix[0][2] * point.x + matrix[1][2] * point.y +
			matrix[2][2] * point.z + matrix[3][2] * point.w;
	res.w = matrix[0][3] * point.x + matrix[1][3] * point.y +
			matrix[2][3] * point.z + matrix[3][3] * point.w;

	return res;
}

class Line {
  public:
	Line() {};
	Line(vec3 *a, vec3 *b) {
		start = a;
		end = b;
		length = std::sqrt(std::pow(a->x - b->x, 2) + std::pow(a->y - b->y, 2) +
						   std::pow(a->z - b->z, 2));
	}
	vec3 getStart() { return *start; }
	vec3 getEnd() { return *end; }

  private:
	vec3 *start;
	vec3 *end;
	float length;
};

class Line2D {
  public:
	Line2D() {};
	Line2D(vec2 a, vec2 b) {
		start = a;
		end = b;
		length = std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
	}

	std::vector<vec2> getPoints() {
		std::vector<vec2> points;

		int x0 = start.x;
		int y0 = start.y;
		int x1 = end.x;
		int y1 = end.y;

		int dx = std::abs(x1 - x0);
		int dy = -std::abs(y1 - y0);
		int sx = (x0 < x1) ? 1 : -1;
		int sy = (y0 < y1) ? 1 : -1;
		int err = dx + dy;

		while (true) {
			points.push_back(vec2(x0, y0));

			if (x0 == x1 && y0 == y1)
				break;

			int e2 = 2 * err;
			if (e2 >= dy) {
				err += dy;
				x0 += sx;
			}
			if (e2 <= dx) {
				err += dx;
				y0 += sy;
			}
		}
		return points;
	}

  private:
	vec2 start;
	vec2 end;
	float length;
};

class Cube {
  public:
	Cube(float x, float y, float z, float w, float l, float h) {
		verticies = {vec3(x, y, z),			vec3(x + w, y, z),
					 vec3(x, y + l, z),		vec3(x + w, y + l, z),
					 vec3(x, y, z + h),		vec3(x + w, y, z + h),
					 vec3(x, y + l, z + h), vec3(x + w, y + l, z + h)};
		edges = {
			Line(&verticies[0], &verticies[1]),
			Line(&verticies[2], &verticies[3]),
			Line(&verticies[4], &verticies[5]),
			Line(&verticies[6], &verticies[7]),
			Line(&verticies[0], &verticies[2]),
			Line(&verticies[1], &verticies[3]),
			Line(&verticies[4], &verticies[6]),
			Line(&verticies[5], &verticies[7]),
			Line(&verticies[0], &verticies[4]),
			Line(&verticies[1], &verticies[5]),
			Line(&verticies[2], &verticies[6]),
			Line(&verticies[3], &verticies[7]),
		};
	};

	void move(float x, float y, float z) {
		for (auto &point : verticies) {
			point.x += x;
			point.y += y;
			point.z += z;
		}
	}
	void rotate(float x, float y, float z) {
		// Find center
		vec3 center;
		for (auto &v : verticies) {
			center.x += v.x;
			center.y += v.y;
			center.z += v.z;
		}
		center.x /= 8;
		center.y /= 8;
		center.z /= 8;

		matrix4x4 rx = rotateX(x);
		matrix4x4 ry = rotateY(y);
		matrix4x4 rz = rotateZ(z);
		matrix4x4 combined = multMat(rx, ry);
		combined = multMat(combined, rz);

		for (auto &v : verticies) {
			// Shift to origin
			vec4 p(v.x - center.x, v.y - center.y, v.z - center.z, 1.0f);
			vec4 rotated = multVec(p, combined);
			// Shift back
			v.x = rotated.x + center.x;
			v.y = rotated.y + center.y;
			v.z = rotated.z + center.z;
		}
	}

	std::array<vec3, 8> getVerticies() { return verticies; }
	std::array<Line, 12> getEdges() { return edges; }

  private:
	std::array<vec3, 8> verticies;
	std::array<Line, 12> edges;
};

class Camera3D {
  public:
	Camera3D(float fov, float aspect, float near, float far)
		: fov(fov), aspect(aspect), near(near), far(far) {
		float f = 1.0 / std::tan(toRadians(fov) / 2.);
		projection = {{{{f / aspect, 0, 0, 0}},
					   {{0, f, 0, 0}},
					   {{0, 0, (far + near) / (near - far), -1}},
					   {{0, 0, (2 * far * near) / (near - far), 0}}}};
	}

	Line2D project2d(Line line, matrix4x4 &view, float width, float height) {
		vec4 start = vec4(line.getStart());
		vec4 end = vec4(line.getEnd());
		vec4 cam_space = multVec(start, view);

		vec4 clip_space = multVec(cam_space, projection);

		vec2 outStart(0, 0);

		outStart.x =
			clip_space.w != 0 ? clip_space.x / clip_space.w : clip_space.x;
		outStart.y =
			clip_space.w != 0 ? clip_space.y / clip_space.w : clip_space.y;

		outStart.x = (outStart.x + 1.0) * .5 * width;
		outStart.y = (1.0 - outStart.y) * .5 * height;

		cam_space = multVec(end, view);

		clip_space = multVec(cam_space, projection);

		vec2 outEnd(0, 0);

		outEnd.x =
			clip_space.w != 0 ? clip_space.x / clip_space.w : clip_space.x;
		outEnd.y =
			clip_space.w != 0 ? clip_space.y / clip_space.w : clip_space.y;

		outEnd.x = (outEnd.x + 1.0) * .5 * width;
		outEnd.y = (1.0 - outEnd.y) * .5 * height;

		return Line2D(outStart, outEnd);
	}

	vec2 project2d(vec4 &point, matrix4x4 &view, float width, float height) {
		vec4 cam_space = multVec(point, view);

		vec4 clip_space = multVec(cam_space, projection);

		vec2 res(0, 0);

		res.x = clip_space.w != 0 ? clip_space.x / clip_space.w : clip_space.x;
		res.y = clip_space.w != 0 ? clip_space.y / clip_space.w : clip_space.y;

		res.x = (res.x + 1.0) * .5 * width;
		res.y = (1.0 - res.y) * .5 * height;

		return res;
	}
	float fov, aspect, near, far;
	matrix4x4 projection;
};

class Screen {
  public:
	Screen(int width, int height, float fov, float near, float far,
		   float worldScale)
		: width(width), height(height),
		  camera(fov, (width * 1.0f) / (height * 0.5f), near, far) {
		float s = 1.0f / worldScale;
		view = {
			{{{s, 0, 0, 0}}, {{0, s, 0, 0}}, {{0, 0, s, 0}}, {{0, 0, 0, 1}}}};
		clearBuffer();
	};
	int width, height;
	float fov, aspect, near, far;
	matrix4x4 view;
	Camera3D camera;
	std::vector<std::vector<float>> buffer;
	;

	void clearBuffer() {
		buffer = std::vector<std::vector<float>>(
			height, std::vector<float>(width, 0.0f));
	}

	void addToBuffer(vec2 point) { addToBuffer(point.x, point.y); }

	void addToBuffer(float x, float y) {
		if (x > 0 && y > 0 && x < buffer[0].size() - 1 &&
			y < buffer.size() - 1) {
			buffer[std::round(y)][std::round(x)] = 0.9;
		}
	}

	void addToBuffer(float x, float y, float val) {
		if (x > 0 && y > 0 && x < buffer[0].size() - 1 &&
			y < buffer[1].size() - 1) {
			buffer[std::round(y)][std::round(x)] = val;
		}
	}

	void draw() {
		std::system("clear");
		for (const auto &line : buffer) {
			for (float val : line) {
				{
					std::cout << getDrawChar(val);
				}
			}
			std::cout << std::endl;
		}
	}

	void addPoint(vec3 point) {
		vec4 pointw(point);
		vec2 dp = camera.project2d(pointw, view, width, height);
		addToBuffer(dp);
	}

	void addLine(Line2D line) {
		for (auto point : line.getPoints()) {
			addToBuffer(point);
		}
	}

	void addLine(Line line3D) {
		Line2D line = camera.project2d(line3D, view, width, height);
		addLine(line);
	}
};

int main() {
	const std::chrono::nanoseconds frameDuration(1000000000 / 60);
	auto nextFrame = std::chrono::steady_clock::now();
	int frameCount = 0;
	Screen screen(127, 72, 81, 0.1f, 1000.f, 20);
	Cube cube(0, 0, 9, 5, 5, 5);
	while (true) {
		nextFrame += frameDuration;

		screen.clearBuffer();

		for (const auto line : cube.getEdges()) {
			screen.addLine(line);
		}

		screen.draw();

		cube.rotate(0, 0.5, 0);
		cube.move(0, 0.01, 0);

		std::this_thread::sleep_until(nextFrame);
		frameCount++;
	}
	return 0;
}
