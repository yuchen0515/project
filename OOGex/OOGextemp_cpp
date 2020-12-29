// OOGex.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <iomanip>
#include "mcts.h"
#include <time.h>
#include <random>

#define PRINT

using namespace std;
int simLimit = 10000;   //¿¿¿¿
int p1Wins, p2Wins, draw;

void MvsM(){
	State state;
	int step = 1;
	int r = rand() % MNK::G_SIZE;
	state.do_move(r);   //¿¿¿¿¿¿
	//state.do_move(112);
	//state.do_move(38);
	//state.do_move(199);
	//state.do_move(53);
	//state.do_move(200);
    
#ifdef PRINT
	cout << state << endl;
	double startTime, finishTime;
#endif


	while (!state.is_end()) {
#ifdef PRINT
		startTime = clock();
#endif
        //¿¿
		if (step % 2 == 0){ 
			auto move = PURE::MCTS(state, simLimit);
#ifdef PRINT
			cout << "Output move = " << (char)(move % 15 + 65) << setw(2) << (15 - move / 15) << endl;
#endif
			state.do_move(move);
		}
        //¿¿
		else{ 
			auto move = MCTSS::MCTS(state, simLimit);
#ifdef PRINT
			cout << "Output move = " << (char)(move % 15 + 65) << setw(2) << (15 - move / 15) << endl;
#endif
			state.do_move(move);
		}

		step++;

#ifdef PRINT
		finishTime = clock();
		cout << "Time spent: " << (finishTime - startTime) / CLOCKS_PER_SEC << " s" << endl;
		cout << state << endl;
#endif
		
	}

	if (step == MNK::G_SIZE)
		draw++;
	else if (step % 2 == 1)
		p1Wins++;
	else
		p2Wins++;
	cout << state << endl;
#ifdef PRINT
	cout << "Game Over!" << endl;
	cout << "=====================================" << endl;
#endif
}


int _tmain(int argc, _TCHAR* argv[])
{
	int games = 1; //¿¿¿¿¿¿
	p1Wins = 0;
	p2Wins = 0;

	for (int i = 1; i <= games; i++){
		MvsM();

		cout << "Game " << i << ": Player 1 won " << p1Wins << " games, Player 2 won " << p2Wins << " games." << endl;
	}

	system("PAUSE");
	return 0;
}

