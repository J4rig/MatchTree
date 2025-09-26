#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <cmath>
#include <vector>

using namespace std;

constexpr int n = 5;

Color colors[n] = { RED, GREEN, BLUE, YELLOW, PURPLE };

enum class STATE {
	SCAN = 0,
	DELETE,
	SHIFT,
	FILL
};

class Tile {
public:
	int value = 0;
	bool matched = false;
	
	Tile() {
		value = rand() % n;
	}

	Tile(int value) : value(value) {}
};




class Board {
private:
	typedef vector<shared_ptr<Tile>> Row;
	typedef vector<shared_ptr<Row>> Grid;
	int scan_counter = 0;

public:
	Vector2 dimensions = { 0,0 };
	shared_ptr<Grid> grid = make_shared<Grid>();
	Vector2 selectedTileCoordinates = { 0,0 };
	int n_of_matched = 0;
	Vector2 offset = { 0,0 };
	Vector2 tile_size = { 10,10 };
	int tile_space = 2;
	STATE state = STATE::SCAN;

	Board(Vector2 dimensions, Vector2 offset, Vector2 tile_size, int tile_space) :
		dimensions(dimensions), offset(offset), tile_size(tile_size), tile_space(tile_space) 
	{
		for (int y = 0; y < dimensions.y; y++) {
			grid->emplace_back(make_shared<Row>());
			for (int x = 0; x < dimensions.x; x++) {
				(*grid)[y]->emplace_back(make_shared<Tile>());
			}
		}
	}

	int getTileValue(Vector2 coordinates) {
		if (coordinates.x < dimensions.x && coordinates.y < dimensions.y) {
			return (*(*grid)[coordinates.y])[coordinates.x]->value;
		}
		return -1;
	}

	void setTileValue(Vector2 coordinates, int value) {
		if (value < n) (*(*grid)[coordinates.y])[coordinates.x]->value = value;
	}

	shared_ptr<Tile> getTile(Vector2 coordinates) {
		if (coordinates.x < dimensions.x && coordinates.y < dimensions.y) {
			return (*(*grid)[coordinates.y])[coordinates.x];
		}
		return nullptr;
	}

	void print() {
		for (shared_ptr<Row> row : *grid) {
			for (shared_ptr<Tile> tile : *row) {
				cout << tile->value << " ";
			}
			cout << "\n";
		}
	}

	void draw() {
		Vector2 pos = { 0,0 };
		for (shared_ptr<Row> row : *grid) {
			for (shared_ptr<Tile> &tile : *row) {
				if (tile == nullptr) {
					DrawRectangle(offset.x + pos.x, offset.y + pos.y, tile_size.x, tile_size.y, BLACK);
				}
				else {
					DrawRectangle(offset.x + pos.x, offset.y + pos.y, tile_size.x, tile_size.y, colors[tile->value]);
					if (tile->matched) {
						DrawCircle(offset.x + pos.x + tile_size.x / 2 + tile_space / 2, offset.y + pos.y + tile_size.y / 2 + tile_space / 2, tile_size.x / 4, WHITE);
					}
				}
				pos.x += tile_size.x + tile_space;
			}
			pos.x = 0;
			pos.y += tile_size.y + tile_space;
		}
	}

	void scan() {
		for (int y = 0; y < dimensions.y; y++) {
			for (int x = 0; x < dimensions.x; x++) {
				selectedTileCoordinates = { (float)x,(float)y };
				int length = evaluateRow();
				if (length >= 3) markRow(length);
				length = evaluateColumn();
				if (length >= 3) markColumn(length);
			}
		}
	}

	int evaluateRow() {
		int x = selectedTileCoordinates.x + 1;
		int l = 1;
		while (x < dimensions.x) {
			if ((*(*grid)[selectedTileCoordinates.y])[selectedTileCoordinates.x]->value == (*(*grid)[selectedTileCoordinates.y])[x]->value) {
				l++;
				x++;
			}
			else break;
		}
		return l;
	}

	int evaluateColumn() {
		int y = selectedTileCoordinates.y + 1;
		int l = 1;
		while (y < dimensions.y) {
			if ((*(*grid)[selectedTileCoordinates.y])[selectedTileCoordinates.x]->value == (*(*grid)[y])[selectedTileCoordinates.x]->value) {
				l++;
				y++;
			}
			else break;
		}
		return l;
	}

	void markRow(int length) {
		for (int i = 0; i < length; i++) {
			if (!(*(*grid)[selectedTileCoordinates.y])[selectedTileCoordinates.x + i]->matched) {
				n_of_matched++;
				(*(*grid)[selectedTileCoordinates.y])[selectedTileCoordinates.x + i]->matched = true;
			}	
		}
	}

	void markColumn(int length) {
		for (int i = 0; i < length; i++) {
			if (!(*(*grid)[selectedTileCoordinates.y + i])[selectedTileCoordinates.x]->matched) {
				n_of_matched++;
				(*(*grid)[selectedTileCoordinates.y + i])[selectedTileCoordinates.x]->matched = true;
			}
		}
	}

	void deleteMatches() {
		for (shared_ptr<Row> row : *grid) {
			for (shared_ptr<Tile> &tile : *row) {
				if (tile->matched) {
					tile = nullptr;
				}
			}
		}
	}

	void shiftTiles() {
		for (int y = dimensions.y - 1; y >= 0; y--) {
			for (int x = dimensions.x - 1; x >= 0; x--) {
				if ((*(*grid)[y])[x] == nullptr) {
					int type = getUpperTileValue({ (float)x,(float)y });
					if (type != -1) {
						(*(*grid)[y])[x] = make_shared<Tile>(type);
					}
				}
			}
		}
	}

	int getUpperTileValue(Vector2 coordinates) {
		int y = coordinates.y - 1;
		while (y >= 0) {
			if ((*(*grid)[y])[coordinates.x] != nullptr) {
				int value = (*(*grid)[y])[coordinates.x]->value;
				(*(*grid)[y])[coordinates.x] = nullptr;
				return value;
			}
			y--;
		}
		return -1;
	}

	void fillHoles() {
		for (int x = 0; x < dimensions.x; x++) {
			if ((*(*grid)[0])[x] == nullptr) {
				(*(*grid)[0])[x] = make_shared<Tile>();
				n_of_matched--;
			}
		}
	}

	bool canSwap(Vector2 coordinates) {
		int value = getTileValue(coordinates);
		cout<< "Coordinates: [" << coordinates.x << " " << coordinates.y << "] " << value << "\n";

		if (coordinates.x + 2 < dimensions.x && coordinates.x >= 0) { //horizontal left
			cout << "horizontal left :[" << coordinates.x << " " << coordinates.y << "]\n";
			cout << value << " ";
			cout << getTileValue({ coordinates.x + 1, coordinates.y }) << " ";
			cout << getTileValue({ coordinates.x + 2, coordinates.y }) << "\n";
			if (value == getTileValue({ coordinates.x + 1, coordinates.y }) &&
				value == getTileValue({ coordinates.x + 2, coordinates.y })) {
				return true;
			}
		}
		if (coordinates.x + 1 < dimensions.x && coordinates.x - 1 >= 0) { //horizontal middle
			cout << "horizontal middle :[" << coordinates.x - 1 << " " << coordinates.y << "]\n";	
			cout << getTileValue({ coordinates.x - 1, coordinates.y }) << " ";
			cout << value << " ";
			cout << getTileValue({ coordinates.x + 1, coordinates.y }) << "\n";
			if (value == getTileValue({ coordinates.x - 1, coordinates.y }) &&
				value == getTileValue({ coordinates.x + 1, coordinates.y })) {
				return true;
			}
		}
		if (coordinates.x < dimensions.x && coordinates.x - 2 >= 0) { //horizontal right
			cout << "horizontal right :[" << coordinates.x - 2 << " " << coordinates.y << "]\n";
			cout << getTileValue({ coordinates.x - 2, coordinates.y }) << " ";
			cout << getTileValue({ coordinates.x - 1, coordinates.y }) << " ";
			cout << value << "\n";
			if (value == getTileValue({ coordinates.x - 2, coordinates.y }) &&
				value == getTileValue({ coordinates.x - 1, coordinates.y })) {
				return true;
			}
		}
		if (coordinates.y + 2 < dimensions.y && coordinates.y >= 0) { //vertical top
			cout << "vertical top :[" << coordinates.x << " " << coordinates.y << "]\n";
			cout << value << " ";
			cout << getTileValue({ coordinates.x, coordinates.y + 1 }) << " ";
			cout << getTileValue({ coordinates.x, coordinates.y + 2 }) << "\n";
			if (value == getTileValue({ coordinates.x, coordinates.y + 1 }) &&
				value == getTileValue({ coordinates.x, coordinates.y + 2 })) {
				return true;
			}
		}
		if (coordinates.y + 1 < dimensions.y && coordinates.y - 1 >= 0) { //vertical middle
			cout << "vertical middle :[" << coordinates.x << " " << coordinates.y - 1 << "]\n";
			cout << getTileValue({ coordinates.x, coordinates.y - 1}) << " ";
			cout << value << " ";
			cout << getTileValue({ coordinates.x, coordinates.y + 1 }) << "\n";
			if (value == getTileValue({ coordinates.x, coordinates.y - 1}) &&
				value == getTileValue({ coordinates.x, coordinates.y + 1 })) {
				return true;
			}
		}
		if (coordinates.y < dimensions.y && coordinates.y - 2 >= 0) { //vertical bottom
			cout << "vertical bottom :[" << coordinates.x << " " << coordinates.y - 2 << "]\n";
			cout << getTileValue({ coordinates.x, coordinates.y - 2}) << " ";		
			cout << getTileValue({ coordinates.x, coordinates.y - 1 }) << " ";
			cout << value << "\n";
			if (value == getTileValue({ coordinates.x, coordinates.y - 2}) &&
				value == getTileValue({ coordinates.x, coordinates.y - 1 })) {
				return true;
			}
		}
		cout << "no match\n";
		return false;
	}

	bool inBounds(Vector2 point) {
		if (point.x < 0 || point.x >= dimensions.x ||
			point.y < 0 || point.y >= dimensions.y ){
			cout << "out of bounds\n";
			return false;
		}
		return true;
	}

	bool swapTiles(Vector2 from, Vector2 to) {
		int tmp_value = getTileValue(from);
		getTile({ from.x, from.y })->value = getTileValue({ to.x, to.y });
		getTile({ to.x, to.y })->value = tmp_value;
		if (inBounds(from) && inBounds(to) && (canSwap(from) || canSwap(to))) {
			return true;
		}
		else {
			cout << "swaping back\n";
			int tmp_value = getTileValue(from);
			getTile({ from.x, from.y })->value = getTileValue({ to.x, to.y });
			getTile({ to.x, to.y })->value = tmp_value;
		}
	}

	void updateState() {
		if (state == STATE::SCAN) {
			cout << "SCAN\n";
			scan_counter++;
			scan();
			if (n_of_matched > 0) {
				scan_counter = 0;
				state = STATE::DELETE;
			}
		}
		else if (state == STATE::DELETE) {
			cout << "DLELETE\n";
			deleteMatches();
			state = STATE::SHIFT;
		}
		else if (state == STATE::SHIFT) {
			cout << "SHIFT\n";
			shiftTiles();
			state = STATE::FILL;
		}
		else if (state == STATE::FILL) {
			cout << "FILL remaining: " << n_of_matched << "\n";
			fillHoles();
			if (n_of_matched == 0) state = STATE::SCAN;
			else state = STATE::SHIFT;
		}
	}

	bool isStable() {
		return scan_counter > 1;
	}
};

int main() {
	
	InitWindow(600, 400, "SWEETS BLAST 2000 EXTREME");

	Board board = Board({ 8,10 },{50,50},{20,20},5);
	board.print();

	STATE state = STATE::SCAN;

	Vector2 origin = {};
	Vector2 direction = {};
	Vector2 selected_tile_coordinates = {};

	float time_since_update = 0.0f;
	float update_period = 0.25f;
	
	SetTargetFPS(30);
	while (!WindowShouldClose()) {

		time_since_update += GetFrameTime();
		if (time_since_update >= update_period) {
			board.updateState();
			time_since_update = 0.0f;
		}

		if (board.isStable() && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			selected_tile_coordinates = GetMousePosition();
			selected_tile_coordinates = Vector2Subtract(selected_tile_coordinates, board.offset);
			selected_tile_coordinates = Vector2Divide(selected_tile_coordinates, Vector2AddValue(board.tile_size, board.tile_space));
			if ((int)(selected_tile_coordinates.x * 100.0) % 100 < 80) {
				selected_tile_coordinates.x = floor(selected_tile_coordinates.x);
			}
			else {
				selected_tile_coordinates.x = -1;
			}
			if ((int)(selected_tile_coordinates.y * 100.0) % 100 < 80) {
				selected_tile_coordinates.y = floor(selected_tile_coordinates.y);
			}
			else {
				selected_tile_coordinates.y = -1;
			}
			cout << selected_tile_coordinates.x << " " << selected_tile_coordinates.y << "\n";

			origin = Vector2Multiply(selected_tile_coordinates, Vector2AddValue(board.tile_size,board.tile_space));
			origin = Vector2Add(origin, board.offset);
			origin = Vector2Add(origin, Vector2Divide(board.tile_size, { 2.0,2.0 }));
		}

		if (board.isStable() && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
			if (selected_tile_coordinates.x >= 0 && selected_tile_coordinates.x < board.dimensions.x && selected_tile_coordinates.y >= 0 && selected_tile_coordinates.y < board.dimensions.y) {
				direction = Vector2Subtract(GetMousePosition(), origin);
				if (abs(direction.x) > abs(direction.y) && direction.x > 10) {
					board.swapTiles(selected_tile_coordinates, { selected_tile_coordinates.x + 1,selected_tile_coordinates.y });
					cout << "right\n";
				}
				else if (abs(direction.x) > abs(direction.y) && direction.x < -10) {
					board.swapTiles(selected_tile_coordinates, { selected_tile_coordinates.x - 1,selected_tile_coordinates.y });
					cout << "left\n";
				}
				else if (abs(direction.x) < abs(direction.y) && direction.y > 10) {
					board.swapTiles(selected_tile_coordinates, { selected_tile_coordinates.x,selected_tile_coordinates.y + 1 });
					cout << "down\n";
				}
				else if (abs(direction.x) < abs(direction.y) && direction.y < -10) {
					board.swapTiles(selected_tile_coordinates, { selected_tile_coordinates.x,selected_tile_coordinates.y - 1 });
					cout << "up\n";
				}
			}
			selected_tile_coordinates = { -1,-1 };
		}
		
		BeginDrawing();
		ClearBackground(BLACK);

		board.draw();
		if (selected_tile_coordinates.x != -1 && selected_tile_coordinates.y != -1) {
			DrawCircleV(origin, 5, WHITE);
		}
		
		EndDrawing();
	}
	CloseWindow();
	return 0;
}