#include <fstream>
#include <math.h>
#include <string.h>
#include <iostream>

#define N 1000

using namespace std; 

double EMA(double stocks[], int n)
{
	double upper_sum = 0.0, lower_sum = 0.0,temp = 0.0;
	double k = 1 - (2.0 / (n - 1.0));
	for (int i = 0; i <= n; i++){
		temp = pow(k,i);
		lower_sum += temp;
		upper_sum += stocks[n-i]*temp;
	}
	return upper_sum / lower_sum;
}

void buyStocks(int& owned,double& wallet,double stockPrice) {
	//count how many stocks you can buy and take their value from the wallet
	int max_possible = wallet/stockPrice;
	owned += max_possible;
	wallet -= stockPrice * max_possible;
}

void sellStocks(int& owned, double& wallet, double stockPrice,double percentage) {
	//count how many stocks to sell and add their value to the wallet
	int number_to_sell = owned * percentage;
	owned -= number_to_sell;
	wallet += stockPrice * number_to_sell;
}

void moneyMaker(double* stocks, double* MACD, double* SIGNAL)
{
	int owned_stocks = 1000;
	double starting_value = owned_stocks * stocks[0];
	cout.precision(17);
	cout << "Starting value= " << starting_value  << endl << endl;
	double wallet = 0.0;
	double max_macd = MACD[0];
	bool bought = false;
	bool sold = false;
	for (int i = 0; i < N-35; i++)
	{
		//check if macd crosses over the signal and buy maximal possible number of stocks
		if (MACD[i] > SIGNAL[i] && !bought){
			buyStocks(owned_stocks, wallet, stocks[i]);
			bought = true;	
			sold = false;
		}
		//check if signal crosses over the macd
		else if(MACD[i] < SIGNAL[i] && !sold){
			//sell 10%,60% or 100% depeding on the ratio of current macd value to the current maximal macd value
			if(MACD[i] < 0.2*max_macd) sellStocks(owned_stocks, wallet, stocks[i], 0.6);
			else if ( MACD[i] > 0.8*max_macd) sellStocks(owned_stocks, wallet, stocks[i],1);
			else sellStocks(owned_stocks, wallet, stocks[i], 0.1);
			bought = false;
			sold = true;
		}
		if (MACD[i] > max_macd) max_macd = MACD[i];
	}
	double final_value = owned_stocks * stocks[N-36] + wallet;
	double ratio = final_value / starting_value;
	cout << "Final value= " << final_value << endl;
	cout << "Profit= " << final_value - starting_value << endl;
	cout << "Ratio = " << ratio << endl;
}
int main(int argc, char *argv[])
{
	double stocks[N];
	double MACD[N-26];
	double SIGNAL[N-35];
	bool makeMoney = false;
	int file_index = 0;
	ifstream File;

	if (argc == 3){
		File.open(argv[2]);
		file_index = 2;
		if (strcmp(argv[1], "-makeMoney") == 0) makeMoney = true;
	}
	else if (argc == 2) {
		File.open(argv[1]);
		file_index = 1;
	}
	else return 0;

	//get stock values from file 
	if (File.is_open()) {
		int n = 0;
		//number have of lines have to be equal to defined N
		while (!File.eof() && n < N) {
			File >> stocks[n];
			n++;
		}
		File.close();
		if (n != N ){
			cout << "Input file has to contain defined " << N << " lines";
			return 0;
		}
	}
	else {
		cout <<"Error opening file" << endl;
		return 0;
	}

	//calculate MACD
	for (int i = 0; i < sizeof(MACD) / sizeof(double); i++) {
		MACD[i] = EMA(stocks + i + 26-12, 12) - EMA(stocks + i, 26);
	}
	//calculate signal
	for (int i = 0; i < sizeof(SIGNAL) / sizeof(double); i++) {
		SIGNAL[i] = EMA(MACD + i, 9);
	}

	
	// find index of extension start in path
	int k = 0;
	while (argv[file_index][k] != '.' || argv[file_index][k + 1] != 't') {
		k++;
	}
	char macdtxt[9] = "MACD.txt";
	char signaltxt[11] = "SIGNAL.txt";

	char* macdOutput = new char[k + sizeof(macdtxt)]; 
	char* signalOutput = new char[k + sizeof(signaltxt)];

	//copy default file path
	strncpy(macdOutput, argv[file_index], k);
	strncpy(signalOutput, argv[file_index], k);

	strncpy(macdOutput+k, macdtxt, sizeof(macdtxt)); // += "macd.txt"
	strncpy(signalOutput+k, signaltxt, sizeof(signaltxt)); // +="signal.txt"

	// save macd to file
	ofstream o1(macdOutput);
	for (int i = 0; i < sizeof(MACD) / sizeof(double); i++) {	
		o1 << MACD[i] << endl;
	}
	o1.close();

	// save signal to file
	ofstream o2(signalOutput);
	for (int i = 0; i < sizeof(SIGNAL) / sizeof(double); i++) {
		o2 << SIGNAL[i] << endl;
	}
	o2.close();
	
	delete[](macdOutput);
	delete[](signalOutput);

	// run money making algorithm 
	if (makeMoney) moneyMaker(stocks+35,MACD+9,SIGNAL);
	return 0;
}