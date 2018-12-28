#include <iostream>
#include <memory>
#include <vector>
#include <bitset>
#include <algorithm>
#include <cmath>

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
        std::cerr << static_cast<char>(y + 'A') << static_cast<char>(x + '1') << std::endl;
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
        // should only be used on previously occupied locations
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
        Color currentPlayer; // player that should make his move

        bool operator==(const Board& other) const {
            return (movesPlayed == other.movesPlayed && colors == other.colors && occupied == other.occupied);
        }

		Board() {
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
        Board(Board& board) : occupied(board.occupied), colors(board.colors), currentPlayer(board.currentPlayer), movesPlayed(board.movesPlayed) {
        }

        void flipLocation(Location location) {
            Color oldColor = getColor(location);
            setColor(location.x, location.y, flip(oldColor)); 
        }
		
		void print() {
			for (uint8_t i = 0; i < 8; i++) {
				for (uint8_t j = 0; j < 8; j++) {
					if (!isOccupied(j, i)) {
                        if (doesMoveFlip({Location{j, i}, currentPlayer})) std::cerr << "#";
						else std::cerr << '.';
						continue;
					}
					if (getColor(j, i) == Black) {
						std::cerr << 'b';
					} else {
						std::cerr << 'w';
					}
				}
				std::cerr << std::endl;
			}
            std::cerr << std::endl;
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
        // returns amount of placements written to vector
        std::vector<Location> getPossibleMoves() {
            std::vector<Location> out;
            //TODO: work out if this helps at all:
            // if (matrixIsFilled()) return out;
            bool hasFlippingMove = false;
            for (uint8_t y = 0; y < 8; y++) {
                for (uint8_t x = 0; x < 8; x++) {
                    if (isOccupied(x, y)) continue;
                    Placement p = Placement{Location{x, y}, currentPlayer};
                    if (doesMoveFlip(p)) {
                        hasFlippingMove = true;
                        out.push_back(Location{x, y});
                    }
                }
            }           
            if (!hasFlippingMove) {
                // just place it somewhere next to an old stone
                for (uint8_t y = 0; y < 8; y++) {
                    for (uint8_t x = 0; x < 8; x++) {
                        for (auto &d : directions) {
                            if (isOccupied(x + d.first, y + d.second)) {
                                out.push_back(Location{x, y});
                                break;
                            }
                        }
                    }
                }
            }
            return out;
        }
        
        bool isWinner(Color player) {
            // 30 is always a loss;
            int nbits = std::bitset<64>(colors).count();
            if (player == Black) {
                return nbits > 30;
            } else {
                return nbits < 30;
            }
            return false;
        }
};


class Node {
    public:
        Location move; // the move that got us here
        Node * parent;
        std::vector<Node*> children;
        std::vector<Location> untriedMoves;
        int wonGames = 0;
        int playedGames = 0;
        Color playerToMove;
        
        // root initialization
        Node(Board &state) : parent(NULL) {
            untriedMoves = state.getPossibleMoves();
            playerToMove = state.currentPlayer;
        }

        // child initialization
        Node(Board &state, Node * parent) : parent(parent) {
            untriedMoves = state.getPossibleMoves();
            playerToMove = state.currentPlayer;
        } 

        ~Node() {
            for (auto c : children) {
                delete c;
            }
        }

        // returns child with highest UCB1
        Node * UCTSelectChild() {
            float mx = -1;
            float other;
            float simulations = static_cast<float>(playedGames);
            Node * best;
            for (auto c : children) {
                // std::cout << simulations << std::endl;
                other = c->UCB1(simulations); 
                // std::cout << other << std::endl;
                if (other > mx) {
                    mx = other;
                    best = c;
                }
            }
            // std::cout << mx << std::endl;
            return best;
        }

        float UCB1(float simulations) {
            float pg = float(playedGames);
            try {
                return (float(wonGames) / pg) + sqrt(log(simulations) / pg);
            } catch (...) {
                return 0; // just return 0 if it tries to take the sqrt of a negative number
            }
        }

        void makeChildRoot(Node * newRoot) {
            int n = children.size();
            for (int i = 0; i < n; i++) {
                if (children[i] == newRoot) continue;
                delete children[i];
            }
            children.clear();
        }
};

class Tree {
    private:
        bool selection() {
            while (cursor->untriedMoves.empty()) {
                cursor = cursor->UCTSelectChild();
                state.place(Placement{cursor->move, state.currentPlayer});
                if (state.matrixIsFilled()) return false;
            }            
            return true;
        }
        void expansion() {
            // if (state.matrixIsFilled()) return;
            int index = rand() % cursor->untriedMoves.size();
            Location newMove = cursor->untriedMoves[index];
            state.place(Placement{newMove, state.currentPlayer});
            Node * newChild = new Node(state, cursor);
            newChild->move = newMove;
            cursor->children.push_back(newChild);
            cursor->untriedMoves.erase(cursor->untriedMoves.begin() + index); // erase from untried moves
            cursor = newChild;
        }

        void simulation() {
            Location move;
            while (!state.matrixIsFilled()) {
                move = cursor->untriedMoves[rand() % cursor->untriedMoves.size()];
                state.place(Placement{move, state.currentPlayer});
                Node * newChild = new Node(state, cursor);
                newChild->move = move;
                cursor->children.push_back(newChild);
                cursor->untriedMoves.erase(
                    std::find(
                        cursor->untriedMoves.begin(), 
                        cursor->untriedMoves.end(),
                        move
                    )
                );
                cursor = newChild;
            }
        }

        void backpropagation() {
            if (state.isWinner(playerPiece)) {
                while (cursor != root) {
                    cursor->wonGames++;
                    cursor->playedGames++;
                    cursor = cursor->parent;
                }
                root->playedGames++;
                root->wonGames++;
            } else {
                while (cursor != root) {
                    cursor->playedGames++;
                    cursor = cursor->parent;
                }
                root->playedGames++;
            }
        }

    public:
        Node * root;
        Node * cursor;
        Board rootState;
        Board state;
        Color playerPiece;

        Tree(Color playerPiece) : playerPiece(playerPiece) {
            root = new Node(rootState);
            cursor = root;
        }

        // Only call when at least 1 child
        Node * mostVisitedChild() {
            int n = root->children.size();
            int max = 0;
            Node * best;
            for (int i = 0; i < n; i++) {
                if (root->children[i]->playedGames > max) {
                    max = root->children[i]->playedGames;
                    best = root->children[i];
                }
            }
            return best;
        }

        void makeChildRoot(Node * newRoot) {
            rootState.place(Placement{newRoot->move, rootState.currentPlayer});
            rootState.print();
            root->makeChildRoot(newRoot);
            root = newRoot;
            cursor = root;
        }

        void MCTS(clock_t endTime) {
            do {
                state = Board(rootState);
                if (selection()) expansion();; // only expand if tree isn't already fully expanded
                simulation();
                backpropagation();
            } while (clock() < endTime);
            std::cerr << "Played games: " << root->playedGames << std::endl;
            std::cerr << "Won games: " << root->wonGames << std::endl;
        }

        void advance(Location opponentMove) {
            for (auto c : root->children) {
                if (c->move == opponentMove) {
                    makeChildRoot(c);
                    return;
                }
            }
            std::cerr << "Error: opponent did move that isn't a child" << std::endl;
            exit(1);
        }
};




Location parseString(std::string word) {
    uint8_t rowIndex = static_cast<uint8_t>(word[0] - 'A');
    uint8_t columnIndex = static_cast<uint8_t>(word[1] - '1');
    return Location{columnIndex, rowIndex};
}

int main() {
    Tree t(Black);
    std::string word;
    std::cin >> word;
    clock_t beginTime = clock();
    if (word == "Start") {
        t = Tree(White);
    } else {
        Location l = parseString(word);
        t.state.place(Placement{l, White});
        Node * newRoot = new Node(t.state, t.root);
        newRoot->move = l;
        t.root->children.push_back(newRoot);
        t.makeChildRoot(newRoot);
    }
    const clock_t extraTime = 0.15*CLOCKS_PER_SEC;
    while (!t.rootState.matrixIsFilled()) {
        t.MCTS(beginTime + extraTime);
        Node * bestChild = t.mostVisitedChild();
        bestChild->move.print();
        t.makeChildRoot(bestChild);
        std::cin >> word;
        beginTime = clock();
        t.advance(parseString(word));
    }
    // std::cout << "EOF" << std::endl;
	return 0;
}
