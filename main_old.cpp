// this was my program before I started implementing Monte Carlo Tree Search
#include <iostream>
#include <vector>
#include <bitset>
#include <algorithm>

const std::pair<int, int> directions[8] {
    std::make_pair(0, 1),
    std::make_pair(1, 1),
    std::make_pair(1, 0),
    std::make_pair(1, -1),
    std::make_pair(0, -1),
    std::make_pair(-1, -1),
    std::make_pair(-1, 0),
    std::make_pair(-1, 1),
};

enum Color {
    Black,
    White
};

void printVector(std::vector<int> vec) {
    for (auto &v : vec) std::cout << v << ", ";
    std::cout << std::endl;
}

struct Location {
	uint8_t x;
	uint8_t y;
    bool isInBounds() {
        return (x >= 0 && x < 8 && y >= 0 && y < 8);
    }
    bool operator==(const Location& other) const {
        return (x == other.x && y == other.y);
    }
    bool operator!=(const Location& other) const {
        return (x != other.x || y != other.y);
    }
    void print() {
        std::cout << static_cast<char>(y + 'A') << static_cast<char>(x + '1') << std::endl;
    }
};

struct Placement {
	Location location;
	Color player;
};

Color flip(Color color) {
    if (color == Black) return White;
    else return Black;
}

class Board {
	private:
		uint64_t occupied; // 0 = unoccupied, 1 = occupied
		uint64_t colors; // 0 = white, 1 = black

        // should only be used on unused locations
        // doesn't perform any sort of validation beforehand
        // use carefully
		void set(uint8_t x, uint8_t y, Color piece) {
			occupied |= (1ULL << (8*y + x));
            if (piece == Black)	colors |= (1ULL << (8*y + x));
            //std::cout << std::bitset<64>(colors) << std::endl;
		}

        // overwrites old
        void setColor(uint8_t x, uint8_t y, Color piece) {
            if (piece == Black) { // set bit to 1 if black
                colors |= 1ULL << (8*y + x);
                return;
            }
            colors &= ~(1ULL << (8*y + x)); // reset to 0 if White
        }

		bool isOccupied(uint8_t x, uint8_t y) {
			return (occupied >> (8*y + x)) & 1;
		}

        bool isOccupied(Location location) {
            return isOccupied(location.x, location.y);
        }

		Color getColor(uint8_t x, uint8_t y) {
			if ((colors >> (8*y + x)) & 1) {
                return Black;
            } else {
                return White;
            }
		}

        Color getColor(Location location) {
            return getColor(location.x, location.y);
        }

	public:
        uint8_t movesPlayed; // increases by 1 every move, stop game at 60
		Color playerPiece; // color of monte carlo opponent
        Color currentPlayer; // player that should make his move

		Board(Color playerPiece) : playerPiece{playerPiece} {
			currentPlayer = White; // white starts
			occupied = 0;
            colors = 0;
            movesPlayed = 0;
            // initialize board, white in top left corner
			set(3, 3, White);
			set(4, 4, White);
			set(3, 4, Black);
			set(4, 3, Black);
		}
        
        // clone board
        Board(Board& board) : occupied(board.occupied), colors(board.colors), playerPiece(board.playerPiece), currentPlayer(board.currentPlayer), movesPlayed(board.movesPlayed) {
        }

        void flipLocation(Location location) {
            Color oldColor = getColor(location);
            setColor(location.x, location.y, flip(oldColor)); 
        }
		
		void print() {
			for (uint8_t i = 0; i < 8; i++) {
				for (uint8_t j = 0; j < 8; j++) {
					if (!isOccupied(j, i)) {
                        if (doesMoveFlip({Location{j, i}, currentPlayer})) std::cout << "#";
						else std::cout << '.';
						continue;
					}
					if (getColor(j, i) == Black) {
						std::cout << 'b';
					} else {
						std::cout << 'w';
					}
				}
				std::cout << std::endl;
			}
            std::cout << std::endl;
		}

        bool matrixIsFilled() {
            return movesPlayed >= 60;
        }
		
        // returns true if it's possible to place a piece at location
		bool checkPlacement(Location location) {
			if (location.x < 0 || location.x > 8) return false;
			if (location.y < 0 || location.y > 8) return false;
			if (isOccupied(location.x, location.y)) return false;
			return true;
		}
		
		void place(Placement placement) {
			set(placement.location.x, placement.location.y, placement.player);
            // check every direction and see where the last piece of the same color is
            // then we go in the same direction again and flip every piece
            // until we arrive at the last piece
            for (auto &d : directions) {
                Location currentLocation = placement.location;
                Location lastLocationSameColor = currentLocation;
                while (currentLocation.isInBounds()) {
                    if (!isOccupied(currentLocation)) break;
                    if (getColor(currentLocation) == placement.player) {
                        lastLocationSameColor = currentLocation;
                    }
                    currentLocation.x += d.first;
                    currentLocation.y += d.second;
                }
                currentLocation = placement.location;
                if (lastLocationSameColor == currentLocation) continue;
                currentLocation.x += d.first;
                currentLocation.y += d.second;
                while (currentLocation != lastLocationSameColor) {
                    flipLocation(currentLocation);
                    currentLocation.x += d.first;
                    currentLocation.y += d.second;
                }
            }
            movesPlayed++;
            currentPlayer = flip(currentPlayer);
            // std::cout << currentPlayer << std::endl;
		}

        // check whether a placement makes a single flip or more
        bool doesMoveFlip(Placement placement) {
            for (auto &d : directions) {
                Location currentLocation = placement.location;
                currentLocation.x += d.first;
                currentLocation.y += d.second;
                while (currentLocation.isInBounds()) {
                    if (!isOccupied(currentLocation)) break;
                    if (getColor(currentLocation) == placement.player) {
                        if (abs(currentLocation.x - placement.location.x) <= 1 && abs(currentLocation.y - placement.location.y) <= 1) {
                            currentLocation.x += d.first;
                            currentLocation.y += d.second;
                            continue;
                        }
                        else {
                            return true;
                        }
                    }
                    currentLocation.x += d.first;
                    currentLocation.y += d.second;
                }
            }
            return false;
        }

        // gets possible moves for currentPlayer
        std::vector<Placement> getPossibleMoves() {
            std::vector<Placement> moves;
            bool hasFlippingMove = false;
            for (uint8_t y = 0; y < 8; y++) {
                for (uint8_t x = 0; x < 8; x++) {
                    if (isOccupied(x, y)) continue;
                    Placement p = Placement{Location{x, y}, currentPlayer};
                    if (doesMoveFlip(p)) {
                        hasFlippingMove = true;
                        moves.push_back(p);
                    }
                }
            }           
            if (!hasFlippingMove) {
                // just place it somewhere next to an old stone
                for (uint8_t y = 0; y < 8; y++) {
                    for (uint8_t x = 0; x < 8; x++) {
                        if (isOccupied(x, y)) continue;
                        for (auto &d : directions) {
                            if (isOccupied(x + d.first, y + d.second)) {
                                moves.push_back(Placement{Location{x, y}, currentPlayer});
                                break;
                            }
                        }
                    }
                }
            }
            return moves;
        }

        // monte carlo
        Placement calculateBestMove(clock_t timeEnd) {
            std::vector<Placement> possibleMovesRoot = getPossibleMoves();
            std::vector<int> amountOfWins (possibleMovesRoot.size());
            std::vector<Placement> possibleMoves;
            for (int i = 0; ; i++) {
                int index = rand() % possibleMovesRoot.size();
                Board newBoard (*this);
                newBoard.place(possibleMovesRoot[index]);
                while (!newBoard.matrixIsFilled()) {
                    possibleMoves = newBoard.getPossibleMoves();
                    newBoard.place(possibleMoves[rand() % possibleMoves.size()]);
                }
                int winningPieces = std::bitset<64>(newBoard.colors).count();
                if (currentPlayer == White) winningPieces = 64 - winningPieces;
                if (winningPieces > 32) amountOfWins[index]++;
                if (i % 500 == 0) {
                    // check time, break if no time left
                    if (clock() > timeEnd) break;
                }
            }
            //printVector(amountOfWins);
            return possibleMovesRoot[std::max_element(amountOfWins.begin(), amountOfWins.end()) - amountOfWins.begin()];
        }
};

Location parseString(std::string word) {
    uint8_t rowIndex = static_cast<uint8_t>(word[0] - 'A');
    uint8_t columnIndex = static_cast<uint8_t>(word[1] - '1');
    return Location{columnIndex, rowIndex};
}

int main() {
    Board b(White);
    std::string word;
    std::cin >> word;
    clock_t beginTime = clock();
    if (word == "Start") {
        b = Board(Black);
    } else {
        b.place(Placement{parseString(word), b.playerPiece});
    }
    const clock_t extraTime = 0.13*CLOCKS_PER_SEC;
    while (!b.matrixIsFilled()) {
        Placement p = b.calculateBestMove(beginTime + extraTime);
        p.location.print();
        b.place(p);
        //b.print();
        std::cin >> word;
        beginTime = clock();
        b.place(Placement{parseString(word), b.playerPiece});
    }
	return 0;
}
