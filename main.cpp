//
//  main.cpp
//  Project
//
//  Created by Ashley on 11/16/16.
//  Copyright © 2016 Ashley. All rights reserved.
//

#include "MCTS.hpp"

typedef struct//typedef的作用是在C中定义一个结构体类型
{
    int iFlag;
    int X, Y;
}Point;

class Game
{
public:
    Game();
    
    void draw();
    void playGame();
    //int value(int p,int q);
    //int qXing(int n,int p,int q);    /* 返回空点p q在n方向上的棋型 n为1-8方向 从右顺时针开始数 */
    //void yiwei(int n,int *i,int *j);   /* 在n方向上对坐标 i j 移位 n为1-8方向 从右顺时针开始数  */
    void MakePoint( Point * pPoint, int iGameFlag );
    void AI(int* y, int* x);
    int getWinner(int (*board)[Length], coordinate move); // 1 is BLACK, 2 is WHITE, 0 is "no winner", 3 is TIE
    
    int chessboard[Length][Length];
    int g_iPointLen;
    int player;
    Point Point1,Point2;
    MCTS *mcts;
};


Game::Game(){
    mcts = new MCTS();
    int i,j;
    g_iPointLen = Length * Length;
    Point1.X = 0;
    Point1.Y = 0;
    Point1.iFlag = BLACK;
    Point2.X = 7;
    Point2.Y = 7;
    Point2.iFlag = WHITE;
    for( i=0; i <Length; ++i )
        for(j=0;j<Length;++j)
            chessboard[i][j] = BLANK;
}
void Game:: MakePoint( Point * pPoint, int iGameFlag )
{
    if( iGameFlag )
    {
        char cor;
        printf("please place your coordinate\n ");
        while( scanf( "%c%d", cor, &pPoint->Y) )
        {
            pPoint->X = cor-65;
            if( ( pPoint->X < 0 || pPoint->X >Length-1 ) || ( pPoint->Y < 0 || pPoint->Y >Length-1 ) )
                printf( "WRONG coordinate!PLEASE INPUT AGAIN：");
            else if( chessboard[pPoint->X][pPoint->Y] )
                printf( "THE PLACE IS ALREADY OCCUPIED!PLEASE INPUT AGAIN：");
            else break;
        }
    }
    
    chessboard[pPoint->X][pPoint->Y] = pPoint->iFlag;
    --g_iPointLen;
    system("cls");
    draw();
    if( iGameFlag == 0 )
    {
        char cox;
        cox=pPoint->X+65;
        printf("The blackputer place at %c%d\n", cox, pPoint->Y);
    }
}
void Game::playGame()
{
    printf("\t\t\tPlease input the coordinate（ex:13H）\n\n\n");
    draw();
    printf("First step please input 1，Second step please input2：");
    int choice;
    while( scanf( "%d", &choice ), choice!=1 && choice!=2 )
        printf( "INPUT ERROR!PLEASE INPUT AGAIN!");
    if( choice == 2 )
        MakePoint( &Point2, 0 );
    choice = 1;
    while( g_iPointLen )
    {
        MakePoint( &Point1, 1 );
        coordinate temp;
        temp.row = Point1.X;
        temp.column = Point1.Y;
        if( getWinner( chessboard, temp )==1 ) {
            printf("YOU WIN!\n");
            return;
            
        }    /* 玩家赢 */
        
        if( choice == 1 )
            
        {
            //player=3-player;
            
            AI( &Point2.Y, &Point2.X );
            
            MakePoint( &Point2, 0 );
            coordinate temp;
            temp.row = Point2.X;
            temp.column = Point2.Y;
            if( getWinner(chessboard,temp)==2)
                
            {               /* 电脑赢 */
                
                printf( "THE whitePUTER WIN!\n" );
                
                return;
                
            }
        }
    }
    printf("draw\n");
}

void Game::AI(int* y, int* x)
{
    //    for (int i=0; i<Length; i++) {
    //        for (int j=0; j<Length; j++) {
    //            printf("chess board [%d][%d] = %d\n", i, j, chessboard[i][j]);
    //        }
    //    }
    coordinate move = mcts->getBestAction(this->chessboard, BLACK, this->g_iPointLen);
    printf("return a coordinate\n");
    *y = move.column;
    *x = move.row;
}
void Game::draw() /* 画棋盘 */
{
    int i,j;
    char p[15][15][4];
    for(j=0;j<15;j++)
        for(i=0;i<15;i++){
            if(chessboard[j][i]==BLANK) strcpy(p[j][i],"  \0");
            if(chessboard[j][i]==BLACK) strcpy(p[j][i],"1\0");
            if(chessboard[j][i]==WHITE) strcpy(p[j][i],"2\0");
        }

    printf("        00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 \n");
    printf("       |--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    
    for(i=0,j=0;i<14;i++,j++)
    {
        printf("      %c|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%c\n",j+65,p[i][0],p[i][1],p[i][2],p[i][3],p[i][4],p[i][5],p[i][6],p[i][7],p[i][8],p[i][9],p[i][10],p[i][11],p[i][12],p[i][13],p[i][14],j+65);
        printf("       |--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    }
    printf("      O|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|%2s|O\n",p[14][0],p[14][1],p[14][2],p[14][3],p[14][4],p[14][5],p[14][6],p[14][7],p[14][8],p[14][9],p[14][10],p[14][11],p[14][12],p[14][13],p[14][14]);
    printf("       |--|--|--|--|--|--|--|--|--|--|--|--|--|--|--|\n");
    printf("        00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 \n");
}

int Game::getWinner(int (*board)[Length],coordinate coor)
{
    const int x = coor.row;
    const int y = coor.column;
    int player = board[x][y];
    int i,w=1,mz=1,nz=1,z=1;
    for(i=1;i<5;i++)
    {
        if(y+i<15&&board[x][y+i]==player)
            w++;
        else break;//下
    }
    for(i=1;i<5;i++)
    {
        if(y-i>=0&&board[x][y-i]==player)
            w++;
        else break;//上
    }
    if(w>=5)
        return player;
    for(i=1;i<5;i++)
    {
        if(x+i<15&&board[x+i][z]==player)
            mz++;
        else break;//右
    }
    for(i=1;i<5;i++)
    {
        if(x-i>=0&&board[x-i][y]==player)
            mz++;
        else break;//左
    }
    if(mz>=5)
        return player;
    for(i=1;i<5;i++)
    {
        if(x+i<15&&y+i<15&&board[x+i][y+i]==player)
            nz++;
        else break;//右下
    }
    for(i=1;i<5;i++)
    {
        if(x-i>=0&&y-i>=0&&board[x-i][y-i]==player)
            nz++;
        else break;//左上
    }
    if(nz>=5)
        return player;
    for(i=1;i<5;i++)
    {
        if(x+i<15&&y-i>=0&&board[x+i][y-i]==player)
            z++;
        else break;//右上
    }
    for(i=1;i<5;i++)
    {
        if(x-i>=0&&y+i<15&&board[x-i][y+i]==player)
            z++;
        else break;//左下
    }
    if(z>=5)
        return player;
    return 0;
}
int main()
{
    char k;
    system("color E0");//设置颜色
    do{
        Game *game = new Game();
        game->playGame();
        printf("Do you want a new turn?input y or n："); getchar(); scanf("%c",&k);
        while(k!='y'&&k!='n'){ printf("INPUT ERROR!\n"); scanf("%c",&k); }
        system("cls");
    }while(k=='y');
    printf("Thanks for using!\n");
    
    return 0;
}
