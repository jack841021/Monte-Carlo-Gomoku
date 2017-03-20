#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <stdlib.h>
#include <math.h>
#include <set>
#include <time.h>
#define BLACK 1
#define WHITE 2
#define BLANK 0
#define Length 15
using namespace std;

typedef struct coordinate
{
    int row;
    int column;
} coordinate;

int temp_board[Length][Length];

bool operator < (coordinate const & x, coordinate const & y)
{
    return (x.row >= 0 && y.row >= 0);
}

bool operator == (coordinate const & x, coordinate const & y)
{
    if (x.row == y.row && x.column == y.column)
    {
        return -1;
    }
    else if ((x.row < y.row) && (x.column < y.column))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

class Node
{
public:
    Node *parent = NULL;
    vector<Node*> children;
    int wins = 0;
    int visits = 0;
    coordinate action;
    int player = -1;
    int depth = 0;
    int state[15][15] = {{0}};
    int number_of_chess;
    Node(Node* parent, coordinate action, int player, int depth, int (*state)[Length], int number_of_chess);
    string ToString();
};

class MCTS
{
public:
    Node* Selection(Node* current, int (*board)[Length]);
    Node* Expand(Node* current, int (*board)[Length]);
    int Simulate(Node* current, int (*board)[Length], int player);
    void BackPropagation(Node* current, int value);
    Node* getBestChild(Node* current, int Cp); // by UCT
    coordinate getBestAction(int (*board)[Length], int player, int g_iPointLen);
    int opponent(int startPlayer);
    set<coordinate> getValidMoves(const int (*state)[Length], int player);
    bool isTerminal(int (*board)[Length], coordinate move);
    int getWinner(int (*board)[Length], coordinate move);
    void mark(int (*board)[Length], int player, coordinate move);
};

void manager(const int (*board)[Length], int player);
void grader();
void multiplier(int player);
set<coordinate> converter();

Node* MCTS::getBestChild(Node* current, int Cp)
{
    Node *bestChild = NULL;
    double bestUBT = -1000;
    vector<Node*> child = current->children;
    vector<Node*>::const_iterator child_iterator;
    vector<Node*> bestChildren;
    for(child_iterator = child.begin(); child_iterator != child.end(); child_iterator++)
    {
        double UBT = ((*child_iterator)->wins / (*child_iterator)->visits) + 2*Cp*sqrt((2*log(current->visits))/(*child_iterator)->visits);
        if (UBT > bestUBT)
        {
            bestUBT = UBT;
            bestChildren.clear();
            bestChildren.push_back(*child_iterator);
        }
        else if (UBT == bestUBT)
        {
            bestChildren.push_back(*child_iterator);
        }
    }
    unsigned seed;
    seed = (unsigned)time(NULL); // 取得時間序列
    srand(seed); // 以時間序列當亂數種子
    int r = rand() % bestChildren.size();
    bestChild = bestChildren[r];
    return bestChild;
}

coordinate MCTS::getBestAction(int (*board)[Length], int player, int g_iPointLen)
{
    Node *root = new Node(NULL, *new coordinate, player, 0, board, Length*Length-g_iPointLen);
    for(int i=0; i<1000; i++)
    {
        Node *current = Selection(root, board);
        int value = Simulate(current, board, opponent(player));
        BackPropagation(current, value);
    }
    return getBestChild(root, 0)->action;
}

int MCTS::opponent(int startPlayer)
{
    if(startPlayer == BLACK)
        return WHITE;
    else
        return BLACK;
}

Node* MCTS::Selection(Node* current, int (*board)[Length])
{
    do {
        set<coordinate> validMoves = getValidMoves(current->state, current->player);
        if (validMoves.size() > current->children.size())
        {
            Node *node = Expand(current, board);
            return node;
        }
        else
        {
            double Cp = 1.44;
            current = getBestChild(current, Cp);
        }
    }
    while(!isTerminal(current->state, current->action) || current->number_of_chess>0);
    return current;
}

Node* MCTS::Expand(Node* current, int (*board)[Length])
{
    set<coordinate> validMoves = getValidMoves(current->state, current->player);
    set<coordinate>::const_iterator move_iterator;
    vector<Node*>::const_iterator iterator;
    for (iterator = current->children.begin(); iterator!=current->children.end(); iterator++)
    {
        coordinate temp;
        temp.row = (*iterator)->action.row;
        temp.column = (*iterator)->action.column;
        move_iterator = find(validMoves.begin(), validMoves.end(), temp);
        validMoves.erase(move_iterator);
    }
    move_iterator = validMoves.begin();
    int playerActing = opponent(current->player);
    Node *node = new Node(current, *move_iterator, playerActing, current->depth+1, current->state, current->number_of_chess+1);
    current->children.push_back(node);
    mark(node->state, playerActing, *move_iterator);
    return node;
}

int MCTS::Simulate(Node* current, int (*board)[Length], int startPlayer)
{
    int temp_board[Length][Length];
    int number_of_chess = current->number_of_chess;
    std::copy(&(current->state[0][0]), &(current->state[0][0]) + 15*15, &temp_board[0][0]);
    if(getWinner(temp_board, current->action) == opponent(startPlayer))
    {
        // not good
        // current->parent->wins = -1;
        return 0;
    }
    if (number_of_chess==225)
    {
        // the game is tie
        return 1;
    }
    int player = opponent(current->player);
    coordinate move = current->action;
    int winner = getWinner(temp_board, move);
    unsigned seed;
    seed = (unsigned)time(NULL); // 取得時間序列
    srand(seed); // 以時間序列當亂數種子
    while( winner == 0 && number_of_chess<225)
    {
        set<coordinate> moves = getValidMoves(temp_board, player);
        int r = rand() % moves.size();
        set<coordinate>::const_iterator move = moves.begin();
        advance(move, r);
        mark(temp_board, player, *move);
        player = opponent(player);
        number_of_chess++;
        winner = getWinner(temp_board, *move);
    }
    if (winner == startPlayer)
        return 1;
    else if (winner == opponent(startPlayer))
    {
        return 0;
    }
    else
    {
        // tie
        return 1;
    }
}

void MCTS::BackPropagation(Node* current, int value)
{
    do
    {
        current->visits++;
        current->wins += value;
        current = current->parent;
    }
    while (current != NULL); // ?
}

void MCTS::mark(int (*board)[Length], int player, coordinate move)
{
    int cordinate_x = move.row;
    int cordinate_y = move.column;
    board[cordinate_x][cordinate_y] = player;
}

int MCTS::getWinner(int (*board)[Length],coordinate coor)
{
    int x = coor.row;
    int y = coor.column;
    int player = board[x][y];
    int i,w=1,mz=1,nz=1,z=1;
    for(i=1;i<5;i++)
    {
        if(y+i<15&&board[x][y+i]==player)
            w++;
        else break; // 下
    }
    for(i=1;i<5;i++)
    {
        if(y-i>=0&&board[x][y-i]==player)
            w++;
        else break; // 上
    }
    if(w>=5)
        return player;
    for(i=1;i<5;i++)
    {
        if(x+i<15&&board[x+i][z]==player)
            mz++;
        else break; // 右
    }
    for(i=1;i<5;i++)
    {
        if(x-i>=0&&board[x-i][y]==player)
            mz++;
        else break; // 左
    }
    if(mz>=5)
        return player;
    for(i=1;i<5;i++)
    {
        if(x+i<15&&y+i<15&&board[x+i][y+i]==player)
            nz++;
        else break; // 右下
    }
    for(i=1;i<5;i++)
    {
        if(x-i>=0&&y-i>=0&&board[x-i][y-i]==player)
            nz++;
        else break; // 左上
    }
    if(nz>=5)
        return player;
    for(i=1;i<5;i++)
    {
        if(x+i<15&&y-i>=0&&board[x+i][y-i]==player)
            z++;
        else break; // 右上
    }
    for(i=1;i<5;i++)
    {
        if(x-i>=0&&y+i<15&&board[x-i][y+i]==player)
            z++;
        else break; // 左下
    }
    if(z>=5)
        return player;
    return 0;
}

bool MCTS::isTerminal(int (*board)[Length], coordinate coor)
{
    if(getWinner(board, coor)==0)
        return true; // 游戏尚未结束
    else
        return false; // 游戏结束
}

set<coordinate> MCTS::getValidMoves(const int (*state)[Length], int player)
{
    manager(state, player);
    grader();
    multiplier(player);
    return converter();
}

Node::Node(Node* parent, coordinate action, int PlayerTookAction, int depth, int (*state)[Length], int number_of_chess)
{
    this->parent = parent;
    this->action = action;
    this->player = (int)PlayerTookAction;
    this->depth = depth;
    std::copy(&state[0][0], &state[0][0] + 15*15, &(this->state[0][0]));
    //this->state[action.row][action.column] = PlayerTookAction;
    this->number_of_chess = number_of_chess;
}

string Node::ToString()
{
    if (parent == NULL)
        return "Root Node";
    stringstream information;
    information << "Action: [" << action.row << "," << action.column << "], wins/visits: " << wins << "/" << visits << ", Took action:  p" << player << ", depth: " << depth;
    return information.str();
}

void manager(const int (*board)[Length], int player)
{
    int i, j;
    
    std::copy(&board[0][0], &board[0][0] + Length*Length, &temp_board[0][0]);
    
    if (player == 1)
    {
        for (i = 0; i <= 14; i++)
        {
            for (j = 0; j <= 14; j++)
            {
                if (temp_board[i][j] == 1)
                {
                    temp_board[i][j] = -1;
                }
                else if (temp_board[i][j] == 2)
                {
                    temp_board[i][j] = -2;
                }
            }
        }
    }
    else if (player == 2)
    {
        for (i = 0; i <= 14; i++)
        {
            for (j = 0; j <= 14; j++)
            {
                if (temp_board[i][j] == 2)
                {
                    temp_board[i][j] = -1;
                }
                else if (temp_board[i][j] == 1)
                {
                    temp_board[i][j] = -2;
                }
            }
        }
    }
}

void grader()
{
    int i, j, n, m, j_, i_, _j;
    
    for (i = 0; i <= 14; i++)
    {
        for (j = 0; j <= 14; j++)
        {
            if (temp_board[i][j] <0)
            {
                for (n = -3; n <= 3; n++)
                {
                    j_ = j + n;
                    
                    i_ = i + n;
                    
                    _j = j - n;
                    
                    m = 4 - abs(n);
                    if (j_ >= 0 and j_ <= 14 and temp_board[i ][j_] >= 0)
                    {
                        temp_board[i ][j_] += m;
                    }
                    if (i_ >= 0 and i_ <= 14 and temp_board[i_][j ] >= 0)
                    {
                        temp_board[i_][j ] += m;
                    }
                    if (i_ >= 0 and i_ <= 14 and j_ >= 0 and j_ <= 14 and temp_board[i_][j_] >= 0)
                    {
                        temp_board[i_][j_] += m;
                    }
                    if (i_ >= 0 and i_ <= 14 and _j >= 0 and _j <= 14 and temp_board[i_][_j] >= 0)
                    {
                        temp_board[i_][_j] += m;
                    }
                }
            }
        }
    }
}

set<coordinate> converter()
{
    set<coordinate> four1;
    set<coordinate> four2;
    set<coordinate> coordinates;
    int i, j, p = 0, q = 0;
    
    for (i = 0; i <= 14; i++)
    {
        for (j = 0; j <= 14; j++)
        {
            if (temp_board[i][j] > p && temp_board[i][j] < 96)
            {
                q = p;
                p = temp_board[i][j];
            }
        }
    }
    
    for (i = 0; i <= 14; i++)
    {
        for (j = 0; j <= 14; j++)
        {
            if (temp_board[i][j] >= 384)
            {
                coordinate a;
                a.row = i;
                a.column = j;
                four1.insert(a);
            }
            if (temp_board[i][j] >= 96)
            {
                coordinate a;
                a.row = i;
                a.column = j;
                four2.insert(a);
            }
            if (temp_board[i][j] == p)
            {
                coordinate a;
                a.row = i;
                a.column = j;
                coordinates.insert(a);
            }
            if (temp_board[i][j] == q)
            {
                coordinate a;
                a.row = i;
                a.column = j;
                coordinates.insert(a);
            }
        }
    }
    if (four1.size() > 0)
    {
        return four1;
    }
    else if (four2.size() > 0)
    {
        return four2;
    }
    else
    {
        return coordinates;
    }
}

void multiplier(int player)
{
    int c = 0, d = 0, k = 0, p = 0, p_ = 0;
    
    for (int i = 0; i < 15; i++)
    {
        for (int j = 0; j < 15; j++)
        {
            int row = i;
            int col = j;
            p_ = p;
            if (temp_board[row][col] == -1)
            {
                p = -1;
                c = c + 1;
            }
            else if (temp_board[row][col] == -2)
            {
                p = -2;
                d = d + 1;
            }
            else if (temp_board[row][col] >= 0)
            {
                p = 0;
            }
            if (p != p_ and p_ != 0)
            {
                if (p_ == -1 and c != 0)
                {
                    int f_row = i;
                    int f_col = j - 1 - c;
                    
                    if (f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    if (col < 15 and temp_board[row][col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    c = 0;
                }
                else if (p_ == -2 and d != 0)
                {
                    int f_row = i;
                    int f_col = j - 1 - d;
                    
                    if (f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= d;
                        }
                    }
                    if (col < 15 and temp_board[row][col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= d;
                        }
                    }
                    d = 0;
                }
            }
            k = j;
        }
        if (c != 0)
        {
            temp_board[i][k - c] *= c;
            c = 0;
        }
        if (d != 0)
        {
            temp_board[i][k - d] *= d;
            d = 0;
        }
    }
    
    c = 0, d = 0, p = 0;
    
    for (int j = 0; j < 15; j++)
    {
        for (int i = 0; i < 15; i++)
        {
            int row = i;
            int col = j;
            p_ = p;
            if (temp_board[row][col] == -1)
            {
                p = -1;
                c = c + 1;
            }
            else if (temp_board[row][col] == -2)
            {
                p = -2;
                d = d + 1;
            }
            else if (temp_board[row][col] >= 0)
            {
                p = 0;
            }
            if (p != p_ and p_ != 0)
            {
                if (p_ == -1 and c != 0)
                {
                    int f_row = i - 1 - c;
                    int f_col = j;
                    
                    if (f_row >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    if (row < 15 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    c = 0;
                }
                else if (p_ == -2 and d != 0)
                {
                    int f_row = i - 1 - d;
                    int f_col = j;
                    
                    if (f_row >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= d;
                        }
                    }
                    if (row < 15 and temp_board[row][col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= d;
                        }
                    }
                    d = 0;
                }
            }
            k = i;
        }
        if (c != 0)
        {
            temp_board[k - c][j] *= c;
            c = 0;
        }
        if (d != 0)
        {
            temp_board[k - d][j] *= d;
            d = 0;
        }
    }
    
    c = 0, d = 0, p = 0;
    
    for (int m = 0; m < 14; m++)
    {
        for (int n = 0; n < 15 - m; n++)
        {
            int row = m + n;
            int col = n;
            p_ = p;
            if (temp_board[row][col] == -1)
            {
                p = -1;
                c = c + 1;
            }
            else if (temp_board[row][col] == -2)
            {
                p = -2;
                d = d + 1;
            }
            else if (temp_board[row][col] >= 0)
            {
                p = 0;
            }
            if (p != p_ and p_ != 0)
            {
                if (p_ == -1 and c != 0)
                {
                    int f_row = m + n - 1 - c;
                    int f_col = n - 1 - c;
                    
                    if (f_row >= 0 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    if (row < 15 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= c;
                        }
                    }
                    c = 0;
                }
                else if (p_ == -2 and d != 0)
                {
                    int f_row = m + n - 1 - d;
                    int f_col = n - 1 - d;
                    
                    if (f_row >= 0 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= d;
                        }
                    }
                    if (row < 15 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= d;
                        }
                    }
                    d = 0;
                }
            }
            k = n;
        }
        if (c != 0)
        {
            temp_board[m + k - c][k - c] *= c;
            c = 0;
        }
        if (d != 0)
        {
            temp_board[m + k - d][k - d] *= d;
            d = 0;
        }
    }
    
    c = 0, d = 0, p = 0;
    
    for (int m = 1; m < 14; m++)
    {
        for (int n = 0; n < 15 - m; n++)
        {
            int row = n;
            int col = m + n;
            p_ = p;
            if (temp_board[row][col] == -1)
            {
                p = -1;
                c = c + 1;
            }
            else if (temp_board[row][col] == -2)
            {
                p = -2;
                d = d + 1;
            }
            else if (temp_board[row][col] >= 0)
            {
                p = 0;
            }
            if (p != p_ and p_ != 0)
            {
                if (p_ == -1 and c != 0)
                {
                    int f_row = n - 1 - c;
                    int f_col = m + n - 1 - c;
                    
                    if (f_row >= 0 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    if (row < 15 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= c;
                        }
                    }
                    c = 0;
                }
                else if (p_ == -2 and d != 0)
                {
                    int f_row = n - 1 - d;
                    int f_col = m + n - 1 - d;
                    
                    if (f_row >= 0 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= d;
                        }
                    }
                    if (row < 15 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= d;
                        }
                    }
                    d = 0;
                }
            }
            k = n;
        }
        if (c != 0)
        {
            temp_board[k - c][m + k - c] *= c;
            c = 0;
        }
        if (d != 0)
        {
            temp_board[k - d][m + k - d] *= d;
            d = 0;
        }
    }
    
    c = 0, d = 0, p = 0;
    
    for (int m = 0; m < 14; m++)
    {
        for (int n = 0; n < 15 - m; n++)
        {
            int row = 14 - n;
            int col = m + n;
            p_ = p;
            if (temp_board[row][col] == -1)
            {
                p = -1;
                c = c + 1;
            }
            else if (temp_board[row][col] == -2)
            {
                p = -2;
                d = d + 1;
            }
            else if (temp_board[row][col] >= 0)
            {
                p = 0;
            }
            if (p != p_ and p_ != 0)
            {
                if (p_ == -1 and c != 0)
                {
                    int f_row = 14 - n + 1 + c;
                    int f_col = m + n - 1 - c;
                    
                    if (f_row < 15 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    if (row >= 0 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= c;
                        }
                    }
                    c = 0;
                }
                else if (p_ == -2 and d != 0)
                {
                    int f_row = 14 - n + 1 + d;
                    int f_col = m + n - 1 - d;
                    
                    if (f_row < 15 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= d;
                        }
                    }
                    if (row >= 0 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= d;
                        }
                    }
                    d = 0;
                }
            }
            k = n;
        }
        if (c != 0)
        {
            temp_board[14 - k + c][m + k - c] *= c;
            c = 0;
        }
        if (d != 0)
        {
            temp_board[14 - k + d][m + k - d] *= d;
            d = 0;
        }
    }
    
    c = 0, d = 0, p = 0;
    
    for (int m = 1; m < 14; m++)
    {
        for (int n = 0; n < 15 - m; n++)
        {
            int row = 14 - m - n;
            int col = n;
            p_ = p;
            if (temp_board[row][col] == -1)
            {
                p = -1;
                c = c + 1;
            }
            else if (temp_board[row][col] == -2)
            {
                p = -2;
                d = d + 1;
            }
            else if (temp_board[row][col] >= 0)
            {
                p = 0;
            }
            if (p != p_ and p_ != 0)
            {
                if (p_ == -1 and c != 0)
                {
                    int f_row = 14 - m - n + 1 + c;
                    int f_col = n - 1 - c;
                    
                    if (f_row < 15 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= c;
                        }
                    }
                    if (row >= 0 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (c == 4)
                        {
                            if (player == 2)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= c;
                        }
                    }
                    c = 0;
                }
                else if (p_ == -2 and d != 0)
                {
                    int f_row = 14 - m - n + 1 + d;
                    int f_col = n - 1 - d;
                    
                    if (f_row < 15 and f_col >= 0 and temp_board[f_row][f_col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[f_row][f_col] *= 64;
                            }
                            else
                            {
                                temp_board[f_row][f_col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[f_row][f_col] *= d;
                        }
                    }
                    if (row >= 0 and col < 15 and temp_board[row][col] >= 0)
                    {
                        if (d == 4)
                        {
                            if (player == 1)
                            {
                                temp_board[row][col] *= 64;
                            }
                            else
                            {
                                temp_board[row][col] *= 16;
                            }
                        }
                        else
                        {
                            temp_board[row][col] *= d;
                        }
                    }
                    d = 0;
                }
            }
            k = n;
        }
        if (c != 0)
        {
            temp_board[14 - m - k + c][k - c] *= c;
            c = 0;
        }
        if (d != 0)
        {
            temp_board[14 - m - k + d][k - d] *= d;
            d = 0;
        }
    }
}
