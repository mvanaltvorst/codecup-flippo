#include <iostream>
#include <memory>
#include <vector>
#include <bitset>
#include <algorithm>
#include <cmath>

#define BIAS 1.0

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
        Board(Board& board) : 
            occupied(board.occupied), 
            colors(board.colors), 
            currentPlayer(board.currentPlayer), 
            movesPlayed(board.movesPlayed) { }

        void flipLocation(Location location) {
            Color oldColor = getColor(location);
            setColor(location.x, location.y, flip(oldColor)); 
        }
		
		// void print() {
        //     std::vector<Location> moves = getPossibleMoves();
		// 	for (uint8_t i = 0; i < 8; i++) {
		// 		for (uint8_t j = 0; j < 8; j++) {
		// 			if (!isOccupied(j, i)) {
        //                 if (std::find(moves.begin(), moves.end(), Location{j, i}) != moves.end()) std::cerr << "#";
		// 				else std::cerr << '.';
		// 				continue;
		// 			}
		// 			if (getColor(j, i) == Black) {
		// 				std::cerr << 'b';
		// 			} else {
		// 				std::cerr << 'w';
		// 			}
		// 		}
		// 		std::cerr << std::endl;
		// 	}
        //     // std::cerr << std::endl;
		// }

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
            out.reserve(30);
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
                Location toTry;
                for (uint8_t y = 0; y < 8; y++) {
                    for (uint8_t x = 0; x < 8; x++) {
                        if (isOccupied(x, y)) continue;
                        for (auto &d : directions) {
                            toTry = Location{x + d.first, y + d.second};
                            if (!toTry.isInBounds()) continue;
                            if (isOccupied(toTry)) {
                                out.push_back(Location{x, y});
                                break;
                            }
                        }
                    }
                }
            }
            return out;
        }
        
        int getReward(Color player) {
            // 30 is always a loss;
            int nbits = std::bitset<64>(colors).count();
            if (nbits == 30) return 1;
            if (player == Black) {
                if (nbits > 30) return 2;
            } else {
                if (nbits < 30) return 2;
            }
            return 0;
        }

        Location getMostGreedyMove(std::vector<Location> &moves) {
            Location mostGreedy;
            Board state;
            int prevMaxStones = 0;

            if (currentPlayer == Black) {
                for (auto move : moves) {
                    state = Board(*this);
                    state.place(Placement{move, Black});
                    int stones = std::bitset<64>(state.colors).count();
                    if (stones > prevMaxStones) {
                        prevMaxStones = stones;
                        mostGreedy = move;
                    }
                }   
            } else {
                for (auto move : moves) {
                    state = Board(*this);
                    state.place(Placement{move, Black});
                    int stones = std::bitset<64>((~state.colors) && state.occupied).count();
                    if (stones > prevMaxStones) {
                        prevMaxStones = stones;
                        mostGreedy = move;
                    }
                }   
            }

            return mostGreedy;
        }
};


class Node {
    public:
        Location move; // the move that got us here
        Node * parent;
        std::vector<Node*> children;
        std::vector<Location> untriedMoves;
        int reward = 0;
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

        // returns child with highest UCT
        Node * UCTSelectChild(bool opponent) {
            float mx = -1;
            float other;
            float logSimulations = log(static_cast<float>(playedGames));
            Node * best;
            for (auto c : children) {
                other = c->UCT(logSimulations, opponent); 
                if (other > mx) {
                    mx = other;
                    best = c;
                }
            }
            return best;
        }

        float UCT(float logSimulations, bool opponent) {
            float pg = float(playedGames);
            try {
                if (opponent) {
                    return ((pg - (float(reward)/2)) / pg) + BIAS*sqrt(logSimulations / pg);
                } else {
                    return (float(reward) / (2*pg)) + BIAS*sqrt(logSimulations / pg);
                }
            } catch (...) {
                std::cerr << "Something wrong in UCT" << std::endl;
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
        // returns true if there's still nodes to be expanded at the end, false if game state is terminal
        bool selection() {
            while (cursor->untriedMoves.empty()) {
                cursor = cursor->UCTSelectChild(state.currentPlayer != playerPiece);
                state.place(Placement{cursor->move, state.currentPlayer});
                if (state.matrixIsFilled()) return false;
            }            
            return true;
        }

        void expansion() {
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
                // move = state.getMostGreedyMove(cursor->untriedMoves);
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
            int bonus = state.getReward(playerPiece);
            while (cursor != root) {
                cursor->reward += bonus;
                cursor->playedGames++;
                cursor = cursor->parent;
            }
            root->playedGames++;
            root->reward += bonus;
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
            // rootState.print();
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
            // for (auto c : root->children) {
            //     //std::cerr << static_cast<char>(c->move.y + 'A') << static_cast<char>(c->move.x + '1') << ' ' << c->UCT(log(static_cast<float>(root->playedGames))) << "\n\tplayed: " << c->playedGames << "\n\twon: " << static_cast<float>(c->reward) / 2 << std::endl;
            //     std::cerr << static_cast<char>(c->move.y + 'A') << static_cast<char>(c->move.x + '1') << "\n\tplayed: " << c->playedGames << "\n\twon: " << static_cast<float>(c->reward) / 2 << std::endl;
            // }
            // std::cerr << "Played games: " << root->playedGames << std::endl;
            // std::cerr << "Won games: " << static_cast<float>(root->reward) / 2 << std::endl;
        }

        void advance(Location opponentMove) {
            for (auto c : root->children) {
                if (c->move == opponentMove) {
                    makeChildRoot(c);
                    return;
                }
            }
            // std::cerr << "Error: opponent did move that isn't a child" << std::endl;
            exit(1);
        }
};




Location parseString(std::string word) {
    uint8_t rowIndex = static_cast<uint8_t>(word[0] - 'A');
    uint8_t columnIndex = static_cast<uint8_t>(word[1] - '1');
    return Location{columnIndex, rowIndex};
}

int main() {
    clock_t absEndTime = clock() + 4.7*CLOCKS_PER_SEC;
    Tree t(Black);
    std::string word;
    std::cin >> word;
    clock_t beginTime = clock();
    if (word == "Start") {
        // std::cerr << "I'm white" << std::endl;
        t = Tree(White);
    } else {
        // std::cerr << "I'm black, other did move " << word << std::endl;
        // std::cerr << "Got board:" << std::endl;
        Location l = parseString(word);
        t.state.place(Placement{l, White});
        Node * newRoot = new Node(t.state, t.root);
        newRoot->move = l;
        t.root->children.push_back(newRoot);
        t.makeChildRoot(newRoot);
    }
    clock_t extraTime;
    while (!t.rootState.matrixIsFilled()) {
        /*if (t.rootState.movesPlayed <= 20) {
            extraTime = extraTime = 0.15*CLOCKS_PER_SEC;
        } else if (t.rootState.movesPlayed > 20 && t.rootState.movesPlayed <= 40) { 
            extraTime = 0.2*CLOCKS_PER_SEC;
        } else {
            if (t.playerPiece == White) {
                extraTime = 2*(absEndTime - beginTime) / (59 - t.rootState.movesPlayed);
            } else {
                extraTime = 2*(absEndTime - beginTime) / (60 - t.rootState.movesPlayed);
            }
        }*/
        
        //if (t.rootState.movesPlayed >= 10 && t.rootState.movesPlayed < 30) {
        if (t.rootState.movesPlayed < 20) {
            extraTime = 0.27*CLOCKS_PER_SEC; // winner after testing: 0.30
        } else {
            if (t.playerPiece == White) {
                extraTime = 2*(absEndTime - beginTime) / (59 - t.rootState.movesPlayed);
            } else {
                extraTime = 2*(absEndTime - beginTime) / (60 - t.rootState.movesPlayed);
            }
        }

        // std::cerr << "Doing MCTS for " << static_cast<float>(extraTime)/CLOCKS_PER_SEC << " seconds," << std::endl;
        // std::cerr << "Time left: " << static_cast<float>(absEndTime - clock()) / CLOCKS_PER_SEC << std::endl;
        t.MCTS(beginTime + extraTime);
        Node * bestChild = t.mostVisitedChild();
        // std::cerr << "Calculated best move for me:" << std::endl;
        bestChild->move.print();
        // std::cerr << "New board:" << std::endl;
        t.makeChildRoot(bestChild);
        std::cin >> word;
        // std::cerr << "Got new move from opponent: " << word << std::endl;
        beginTime = clock();
        t.advance(parseString(word));
    }
	return 0;
}
