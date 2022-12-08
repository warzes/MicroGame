#pragma once

#include <array>
#include <queue>

template <typename T>
inline int sign(T value)
{
	return (value > static_cast<T>(0)) - (value < static_cast<T>(0));
}

#pragma region GenTile.h

enum class GenTile
{
	Unused,

	/* Dungeon */
	Floor,
	Corridor,
	Wall,
	ClosedDoor,
	OpenDoor,
	UpStairs,
	DownStairs,

	/* Forest */
	Grass,
	Tree,
	ClosedGate,
	OpenGate,
	Water,
	Bridge,

	/* Cave */
	Dirt,
	CaveWall,
	Lava,
	// Pit,

	/* Spaceship */
	Void,
	VoidWall,
};

struct Room
{
	Room() = default;
	Room(int rectLeft, int rectTop, int rectWidth, int rectHeight) : left(rectLeft), top(rectTop), width(rectWidth), height(rectHeight) { }

	bool intersects(const Room& rectangle) const;
	bool intersects(const Room& rectangle, Room& intersection) const;

	int left = 0;
	int top = 0;
	int width = 0;
	int height = 0;
};

inline bool Room::intersects(const Room& rectangle) const
{
	Room intersection;
	return intersects(rectangle, intersection);
}

inline bool Room::intersects(const Room& rectangle, Room& intersection) const
{
	// Rectangles with negative dimensions are allowed, so we must handle them correctly

	// Compute the min and max of the first rectangle on both axes
	int r1MinX = std::min(left, static_cast<int>(left + width));
	int r1MaxX = std::max(left, static_cast<int>(left + width));
	int r1MinY = std::min(top, static_cast<int>(top + height));
	int r1MaxY = std::max(top, static_cast<int>(top + height));

	// Compute the min and max of the second rectangle on both axes
	int r2MinX = std::min(rectangle.left, static_cast<int>(rectangle.left + rectangle.width));
	int r2MaxX = std::max(rectangle.left, static_cast<int>(rectangle.left + rectangle.width));
	int r2MinY = std::min(rectangle.top, static_cast<int>(rectangle.top + rectangle.height));
	int r2MaxY = std::max(rectangle.top, static_cast<int>(rectangle.top + rectangle.height));

	// Compute the intersection boundaries
	int interLeft = std::max(r1MinX, r2MinX);
	int interTop = std::max(r1MinY, r2MinY);
	int interRight = std::min(r1MaxX, r2MaxX);
	int interBottom = std::min(r1MaxY, r2MaxY);

	// If the intersection is valid (positive non zero area), then there is an intersection
	if ((interLeft < interRight) && (interTop < interBottom))
	{
		intersection = Room(interLeft, interTop, interRight - interLeft, interBottom - interTop);
		return true;
	}
	else
	{
		intersection = Room(0, 0, 0, 0);
		return false;
	}
}

#pragma endregion


#pragma region Direction.h

struct Point
{
	Point() = default;
	Point(int nx, int ny) : x(nx), y(ny) {}

	bool operator==(const Point& p)
	{
		if (x == p.x && y == p.y) return true;
		else return false;
	}

	Point& operator+=(const Point& a) { x += a.x; y += a.y; return(*this); }
	Point& operator-=(const Point& a) { x -= a.x; y -= a.y; return(*this); }

	int x = 0;
	int y = 0;
};

class Direction : public Point
{
public:
	Direction() = default;
	Direction(int nx, int ny) : Point(nx, ny) {}

	Direction left45() const;
	Direction right45() const;

	Direction left90() const;
	Direction right90() const;

public:
	static const Direction None;
	static const Direction N;
	static const Direction NE;
	static const Direction E;
	static const Direction SE;
	static const Direction S;
	static const Direction SW;
	static const Direction W;
	static const Direction NW;

	static const std::array<Direction, 8> All;
	static const std::array<Direction, 4> Cardinal;
	static const std::array<Direction, 4> Diagonal;
};


inline Point operator-(const Point& d, const Point& p)
{
	return { d.x - p.x, d.y - p.y };
}
inline Point operator+(const Direction& d, const Point& p)
{
	return { d.x + p.x, d.y + p.y };
}
inline Point operator+(const Point& p, const Direction& d)
{
	return { d.x + p.x, d.y + p.y };
}
inline Direction operator*(const Direction& d, float f)
{
	return Direction(d.x * (int)f, d.y * (int)f);
}
inline Point operator/(const Point& p, float f)
{
	return Point{ p.x / (int)f, p.y / (int)f };
}

inline std::vector<Point> getLine(const Point& from, const Point& to, bool orthogonalSteps = false)
{
	Point delta = to - from;
	Point primaryIncrement(sign(delta.x), 0);
	Point secondaryIncrement(0, sign(delta.y));
	int primary = std::abs(delta.x);
	int secondary = std::abs(delta.y);

	if (secondary > primary)
	{
		std::swap(primary, secondary);
		std::swap(primaryIncrement, secondaryIncrement);
	}

	std::vector<Point> line;
	Point current = from;
	int error = 0;

	while (true)
	{
		line.emplace_back(current);

		if (current == to)
			break;

		current += primaryIncrement;
		error += secondary;

		if (error * 2 >= primary)
		{
			if (orthogonalSteps)
				line.emplace_back(current);

			current += secondaryIncrement;
			error -= primary;
		}
	}

	return line;
}

#pragma endregion

#pragma region GenMap.h

struct Point;

class GenMap
{
public:
	GenMap(int width, int height);

	bool isInBounds(int x, int y) const;
	bool isInBounds(const Point& pos) const;

	void setTile(int x, int y, GenTile tile);
	void setTile(const Point& pos, GenTile tile);

	GenTile getTile(int x, int y) const;
	GenTile getTile(const Point& pos) const;

	void fill(GenTile tile);

	// TODO: Better names?
	void move(std::vector<GenTile>&& tiles);
	std::vector<GenTile> copy();

public:
	const int width, height;

private:
	std::vector<GenTile> m_tiles;
};

#pragma endregion

#pragma region Generator.h

class GenMap;
class Rng;
struct Point;

class Generator
{
public:
	enum class PathType
	{
		Straight,
		Corridor,
		WindingRoad,
	};

	virtual ~Generator() = default;

	const std::wstring& getName() const;

	void generate(GenMap& map, Rng& rng);

protected:
	void setName(const std::wstring& name);

	// Cellular automata
	// http://www.jimrandomh.org/misc/caves.html
	void generation(int r1cutoff);
	void generation(int r1cutoff, int r2cutoff);
	void generation(const Room& room, int r1cutoff); // REMOVE:
	void generation(const Room& room, int r1cutoff, int r2cutoff); // REMOVE:

	// Remove or connect unconnected regions
	void removeRegions(int removeProb = 100, int minSize = 0);
	void connectRegions(int minSize = 0, PathType type = PathType::Straight, bool allowDiagonalSteps = true);
	void constructBridges(int minSize = 0);

	// Lloyd's algorithm (Voronoi iteration)
	void relaxation(std::vector<Point>& points);
	void connectPoints(std::vector<Point>& points, PathType type = PathType::Straight);

	// Growing tree
	void growMaze(std::vector<Point>& maze, int x, int y, int windingProb);
	void removeDeadEnds(std::vector<Point>& maze);

	//
	void fill(int wallProb);
	void fill(const Room& room, int wallProb); // REMOVE:

	bool canCarve(const Room& room) const;
	void carveRoom(const Room& room);

	void carvePath(const Point& from, const Point& to);
	void carveCorridor(const Point& from, const Point& to);
	void carveCircle(const Point& center, int radius);

	// Used for river generation
	void carveWindingRoad(const Point& from, const Point& to, int perturbation = 10);
	void extendLine(Point& from, Point& to);

	void erode(int iterations);
	void erodeTiles(GenTile from, GenTile to, int r1cutoff);

	void removeWalls(); // remove unused walls

private:
	virtual void onGenerate() = 0;

	int countTiles(GenTile tile, int x, int y) const; // count adjacent tiles

	// private:
protected:
	GenMap* map = nullptr;
	Rng* rng = nullptr;

	int width = 0;
	int height = 0;

	GenTile unused = GenTile::Unused;
	GenTile floor = GenTile::Floor;
	GenTile corridor = GenTile::Corridor;
	GenTile wall = GenTile::Wall;
	GenTile water = GenTile::Water;
	GenTile bridge = GenTile::Bridge;

private:
	std::wstring name;
};

#pragma endregion

#pragma region GenDungeon.h

class Dungeon : public Generator
{
protected:
	void placeDoors(int doorProb);
	void removeCorridors();
};

class ClassicDungeon : public Dungeon
{
private:
	void onGenerate() override;
};

class BSPDugeon : public Dungeon
{
private:
	void onGenerate() override;
};

class RoomsAndMazes : public Dungeon
{
private:
	void onGenerate() override;
};

#pragma endregion