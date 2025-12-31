//Levi Metzger
//11-30-24
//Final Project
//Calculates and creates a debt payoff plan
#include<iostream>
#include<iomanip>
#include<cmath>
#include<fstream>
#include<string>
#include<regex>
#include<vector>
#include<sstream>
using namespace std;

vector<string> input();
vector<vector<string>> extractData(const vector<string>);
vector<string> names(vector<vector<string>>&);
vector<vector<double>> debtInfo(vector<vector<string>>);
double totalMonthlyDebtPayment(vector<vector<double>>);
void swap(vector<double>&, vector<double>&);
void swap(string&, string&);
void sort(vector<string>&, vector<vector<double>>&, int);
bool containsValue(vector<int>, int);
void timeToPayoff(vector<vector<double>>&, double);
double findLongestTime(const vector<vector<double>>);
int width(const vector<string>, const vector<string>);

int main() {
	vector<string> inputVector;//Stores all original input
	vector<vector<string>> dataVector;//a 2D vector for all of the individual data pieces
	vector<string> nameOfDebt;//Stores the names/descriptions of the debt, it is parallel to the rows of the debt table
	vector<string> columnLabels = { "Name/description", "Current balance", "Annual Interest Rate", "Minimum Monthly Payment", "Months until Payoff" };//Labels for each column in the eventual output
	vector<vector<double>> debtTable;//A 2D vector to store all of the other debt information as doubles
	double moneyForDebt;//stores the total monthly amount that will be spent to pay off debt
	int payoffMethod;//to choose the payoff method

	cout << "This program will create a personal debt payoff plan.\n";
	cout << "Input your information as follows: Name of creditor or description of debt, Current debt balance, Annual interest rate, Minimum monthly payment (Example: \"Visa, $4953.76, 12.8%, $193\")\n\n";

	inputVector = input();//Gathers input and stores it in the input vector
	dataVector = extractData(inputVector);//breaks up the original string into each piece of data and places them into a 2D array
	nameOfDebt = names(dataVector);//gets the debt names/descriptions
	debtTable = debtInfo(dataVector);//gets the rest of the data, converted to doubles

	cout << "Enter the total monthly amount you can spend to pay down your debt: ";
	while (!(cin >> moneyForDebt)) {//Input validation to ensure input is a number
		cout << "Incorrect input. You must enter a number (no $ sign): ";
		cin.clear();
		cin.ignore(1000, '\n');
	}
	if (moneyForDebt <= totalMonthlyDebtPayment(debtTable)) {//Checks if the avaiblable money is enough to pay off debt
		cout << "You do not have enough disposable income to meet your minimum payments. You need to find a way to make more money, or tighten your budget to only the essentials.";
	}
	else {
		cout << "\nDebt payoff methods:\n1. Debt Snowball (smallest debt first)\n2. Debt Avalanche (highest interest rate first)\nEnter your selection: ";//Menu for two debt payoff methods
		while (!(cin >> payoffMethod) && (payoffMethod != 1 && payoffMethod != 2)) {//Input validation
			cout << "Invalid entry, you must enter either \"1\" or \"2\"";
			cin.clear();
			cin.ignore(1000, '\n');
		}
		sort(nameOfDebt, debtTable, payoffMethod);//Sorts the vectors according to the payoff method selected
		timeToPayoff(debtTable, moneyForDebt);//calculates and adds the time to pay off each debt to the 2D vector

		ofstream file("Debt Payoff.txt");//create the output file
		file << "Here is your completed debt payoff plan assuming total payments of $" << moneyForDebt << " per month. Make the minimum payments on all debts except the first, and throw all your extra money at it until it is paid off, then continue down the list. You should be debt free in about " << ceil(findLongestTime(debtTable)) << " months.\n\n";

		for (int j = 0; j < columnLabels.size(); j++) {//Write the column labels to the file
			if (j == 0) {
				file << left << setw(width(nameOfDebt, columnLabels) + 2) << columnLabels[j];//finds the widest value in the names column, and sets the width 2 spaces wider
			}
			else {
				file << columnLabels[j] << "  ";//the rest of the column labels will be the widest element in their columns, so they only need two spaces between them
			}
		}
		file << '\n';
		for (int i = 0; i < debtTable.size(); i++) {//this loop will write the table of data to the file
			file << left << setw(width(nameOfDebt, columnLabels) + 2) << nameOfDebt[i];//write the debt names/descriptions to the file
			for (int j = 0; j < debtTable[i].size(); j++) {//iterate through each row in the 2D vector
				ostringstream s;//stringstream object so symbols can be added to the numbers easily
				switch (j) {//a switch statement so that each column can be formatted uniquely
				case 0:
				case 2:
					s << "$" << fixed << setprecision(2) << debtTable[i][j];
					break;//Columns 0 and 2 contain dollar values, so precision is set to 2 and a dollar sign is added
				case 1:
					s << debtTable[i][j] << "%";
					break;//this column holds the interest rates, so precision is unchanged, and a percent sign is added
				case 3:
					s << fixed << setprecision(1) << debtTable[i][j];
					break;//this contains the number of months until payoff, with the precision set to 1
				}
				file << left << setw(columnLabels[j + 1].size() + 2) << s.str();//prints the stringstream to the file, aligned left, with the width set to 2 spaces more than the width of that column's label
			}
			file << '\n';//new line once each row is finished
		}
		file.close();

		cout << "\nYour debt plan has been calculated, the following information has been written to a text file named \"Debt Payoff\":\n\n";

		ifstream ifile("Debt Payoff.txt");
		string s;
		while (getline(ifile, s)) {
			cout << s << '\n';//prints the contents of the file
		}
		ifile.close();
	}

	return 0;
}

vector<string> input() {//Collects user input and returns it in a vector
	vector<string> inputVector;
	string inputString;
	regex pattern("^(x)|(^[^,]+, (\\$)?[0-9]+(\.[0-9]{2})?, [0-9]+(\.[0-9]+)?(%)?), (\\$)?[0-9]+((\.[0-9]{2})?)$");//Sets the pattern that input must follow

	while (true) {
		cout << "Enter info for a debt, if you are finished, enter \"x\": ";//Input prompt
		getline(cin, inputString);//Input collection

		while (!(regex_match(inputString, pattern))) {//Input validation, ensuring that it matches the given pattern
			cout << "Error! Incorrect input format. Re-enter as directed above: ";
			getline(cin, inputString);
		}
		if (inputString != "x") {//Checks for exit value
			inputVector.push_back(inputString);//adds input to vector
		}
		else {
			break;//If exit value exists, loop is ended.
		}
	}
	return inputVector;//returns the vector filled with the input
}
vector<vector<string>> extractData(const vector<string> inputVector) {//splits the input string into each of its components
	vector<vector<string>> individualData;//the vector to return
	string data;//a string variable to hold the input as it is being read
	for (string s : inputVector) {//iterates through the input vector
		vector<string> row;//a vector to hold each row of elements
		stringstream ss;//a stringstream object to manipulate the input
		ss << s;//places each input string in the stringstream
		while (getline(ss, data, ',')) {//gets each piece of input, using a comma delimiter
			row.push_back(data);//puts each piece in the row vector
		}
		individualData.push_back(row);//adds each row to the 2D vector
	}
	return individualData;
}

vector<string> names(vector<vector<string>>& individualData) {//gets the name/description of each debt
	vector<string> nameVector;//the vector to return
	for (vector<string>& row : individualData) {//iterates through the 2D vector
		nameVector.push_back(row[0]);//adds the first element of each row to the name vector
		row.erase(row.begin());//erases the name from the original vector
	}
	return nameVector;
}

vector<vector<double>> debtInfo(vector<vector<string>> individualData) {//gets the rest of the debt information
	vector<vector<double>> debtTable;//the 2D vector to return
	for (vector<string> v : individualData) {//iterates through the original 2D vector
		vector<double> row;//a vector for each row of data
		for (string s : v) {//iterates through each row of data
			string data;//a string variable to hold the elements
			for (char c : s) {//iterates through each string element
				if ((c >= '0' && c <= '9') || c == '.') {
					data.push_back(c);//adds the characters to the new string
				}
			}
			row.push_back(stod(data));//converts the new string to a double and adds it to the row
		}
		debtTable.push_back(row);//adds the row to the 2D vector
	}
	return debtTable;
}

double totalMonthlyDebtPayment(const vector<vector<double>> debtTable) {//calculates the total of the minimum monthly payments
	double total = 0;
	for (vector<double> i : debtTable) {
		total += i[2];//adds each monthly payment to the total
	}
	return total;
}

void swap(vector<double>& a, vector<double>& b) {//swap two vectors, for use in the sort function
	vector<double> temp = a;
	a = b;
	b = temp;
}

void swap(string& a, string& b) {//swap two strings, for use in the sort function
	string temp = a;
	a = b;
	b = temp;
}

void sort(vector<string>& names, vector<vector<double>>& debtTable, int method) {//sorts the vectors according to the payoff method
	switch (method) {
	case 1:
		for (int i = 0; i < (names.size() - 1); i++) {//a selection sort algorithm
			double lowest = debtTable[i][0];//a variable for the lowest debt balance
			int index = 0;

			for (int j = i + 1; j < names.size(); j++) {
				if (debtTable[j][0] < lowest) {//if the row has a lower balance
					lowest = debtTable[j][0];//set lowest to that balance
					index = j;//save the location
				}
			}
			if (index) {//if index has been changed (ie. if the first element is not the lowest value)
				swap(debtTable[i], debtTable[index]);//swap the vectors
				swap(names[i], names[index]);//swap the names
			}
		}
		break;
	case 2:
		for (int i = 0; i < (names.size() - 1); i++) {//selection sort algorithm
			double highest = debtTable[i][1];//a variable for the highest interest rate
			int index = 0;

			for (int j = i + 1; j < names.size(); j++) {
				if (debtTable[j][1] > highest) {
					highest = debtTable[j][1];
					index = j;
				}
			}
			if (index) {
				swap(debtTable[i], debtTable[index]);
				swap(names[i], names[index]);
			}
		}
		break;
	}
}

bool containsValue(vector<int> v, int x) {//checks whether a vector contains a value, for use in the following function
	for (int i : v) {
		if (x == i) {
			return true;
		}
	}
	return false;
}

void timeToPayoff(vector<vector<double>>& debtTable, double income) {
	vector<vector<double>> debtReduction = debtTable;//A new vector that can be modified to simulate debt paydown
	double time;//Length of time to pay down each debt
	double monthlyPmnt;//the total monthly payment that will be put on each debt
	double totalTime = 0.0;//the cumulative time until each debt will be paid off
	vector<int> indexCheck;//a vector for the index values of all debts that get paid off while other debts are being focused on

	for (int i = 0; i < debtReduction.size(); i++) {
		vector<int> smallIndexes;//a vector to hold the index values of small debts that will be paid off during a single "i"
		if (!containsValue(indexCheck, i)) {//if debt i has already been paid off, the loop will skip to its next iteration
			monthlyPmnt = (income - totalMonthlyDebtPayment(debtReduction)) + debtReduction[i][2];//subtracts the total monthly payments from the total money to be spent on debt, then adds back the minimum monthly payment for debt "i", resulting in the total monthly payment on debt "i"
			double rate = debtReduction[i][1] / 1200;//monthly interest rate
			time = -(log(1 - ((debtReduction[i][0] * rate) / monthlyPmnt)) / log(1 + rate));//formula for the number of months until payoff

			for (int j = i + 1; j < debtReduction.size(); j++) {//this will iterate through each remaining debt
				if (!containsValue(indexCheck, j)) {//makes sure the debt hasn't been paid off
					double rate2 = debtReduction[j][1] / 1200;//the monthly interest rate of debt "j"
					double remainingBalance = ((debtReduction[j][0] * pow((1 + rate2), time)) - (debtReduction[j][2] * ((pow((1 + rate2), time) - 1) / rate2)));//calculates the new balance of debt "j" using the remaining balance formula

					if (remainingBalance < 0.0) {//if debt j will be paid off before debt "i" is done. The code from this line through line 300 all deals with this situation.
						indexCheck.push_back(j);//adds its index to the index check vector
						smallIndexes.push_back(j);//and to the small indexes vector
						double time2 = -(log(1 - ((debtReduction[j][0] * rate2) / debtReduction[j][2])) / log(1 + rate2));//time to pay off debt j
						debtTable[j].push_back(time2 + totalTime);//adds its time to the table
						debtReduction[j][2] = 0.0;//sets its monthly payment to 0
					}
					else {
						debtReduction[j][0] = remainingBalance;//if it won't be paid off: update the balance of debt j
					}
				}
			}
			if (smallIndexes.size()) {//if their are any small debts that will be paid off from their minimum payments
				if (smallIndexes.size() > 1) {//if there is more than one, they will need sorted
					for (int x = 0; x < (smallIndexes.size() - 1); x++) {
						double shortest = debtTable[smallIndexes[x]][3];//this sorts the index values from shortest time to longest
						for (int y = x + 1; y < smallIndexes.size(); y++) {
							if (debtTable[smallIndexes[y]][3] < shortest) {
								int temp = smallIndexes[x];
								smallIndexes[x] = smallIndexes[y];
								smallIndexes[y] = temp;
							}
						}
					}
				}
				double time2 = debtTable[smallIndexes[0]][3] - totalTime;//the time paying on debt i at a specific monthly payment
				double totalJTime = 0.0;//the cumulative total time spent paying on the small debts
				for (int j = 0; j < smallIndexes.size(); j++) {
					debtReduction[i][0] = ((debtReduction[i][0] * pow((1 + rate), time2)) - (monthlyPmnt * ((pow((1 + rate), time2) - 1) / rate)));//calculates the balance of debt i when each successive j becomes paid off
					monthlyPmnt += debtTable[smallIndexes[j]][2];//adds the minimum payment on debt j to the monthly payment for i

					totalJTime += time2;//the total time spent paying on the small debts
					if (j < smallIndexes.size() - 1) {
						time2 = (debtTable[smallIndexes[j + 1]][3] - totalTime) - totalJTime;//the total time of the next small debt, minus the time that has already elapsed
					}
				}

				time = totalJTime + (-(log(1 - ((debtReduction[i][0] * rate) / monthlyPmnt)) / log(1 + rate)));//adds the time remaining to pay off debt i, once the last small debt has been paid off from its minimum payments
			}

			totalTime += time;//add each debt's time to totalTime
			debtReduction[i][2] = 0.0;//sets monthly payment of debt "i" to zero
			debtTable[i].push_back(totalTime);//adds the total time until debt "i" will be paid off to the original vector
		}
	}
}

double findLongestTime(const vector<vector<double>> debtTable) {
	double longest = 0.0;
	for (vector<double> v : debtTable) {
		if (v[3] > longest) {
			longest = v[3];
		}
	}
	return longest;
}

int width(const vector<string> nameOfDebt, const vector<string> nameOfColumns) {//calculates the width of the name/description column
	int maxWidth = nameOfColumns[0].size();//sets max width variable equal to the column label width
	for (string s : nameOfDebt) {//iterates through the name vector to see if any of the names are wider than the label
		if (s.size() > maxWidth) {
			maxWidth = s.size();
		}
	}
	return maxWidth;//returns the widest value
}