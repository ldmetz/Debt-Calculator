#include<iostream>
#include<iomanip>
#include<cmath>
#include<fstream>
#include<string>
#include<algorithm>
#include<vector>
#include<sstream>
#include<filesystem>
using namespace std;

struct Debt {
	string name {};
	double balance {};
	double interestRate {};
	double minPayment {};
	double payoffTime {};
};

vector<Debt> getInput();
double sumMinimumPayments(const vector<Debt>&);
bool debtsMaintainable(const vector<Debt>&, double);
void calculatePayoff(vector<Debt>&, double);
void writeDataFile(ofstream&, const vector<Debt>&, const vector<string>&, double);
void printData(ifstream&);

int main() {
	vector<Debt> debts;
	const vector<string> columnLabels = { "Name/description", "Current balance", "Annual Interest Rate", "Minimum Monthly Payment", "Months until Payoff" };//Labels for each column in the eventual output
	double moneyForDebt;//stores the total monthly amount that will be spent to pay off debt
	int payoffMethod;//to choose the payoff method

	cout << "This program will create a personal debt payoff plan.\n";
	debts = getInput();

	cout << "Enter the total monthly amount you can spend on debt payments: ";
	while (!(cin >> moneyForDebt)) {//Input validation to ensure input is a number
		cout << "Incorrect input. You must enter a number (no $ sign): ";
		cin.clear();
		cin.ignore(1000, '\n');
	}

	if (!debtsMaintainable(debts, moneyForDebt)) {//Checks if the avaiblable money is enough to pay off debt
		cout << "You do not have enough disposable income to meet your minimum payments. You need to find a way to make more money, or tighten your budget to only the essentials.";
	}
	else {
		cout << "\nDebt payoff methods:\n1. Debt Snowball (smallest debt first)\n2. Debt Avalanche (highest interest rate first)\nEnter your selection: ";//Menu for two debt payoff methods
		while (!(cin >> payoffMethod) && (payoffMethod != 1 && payoffMethod != 2)) {//Input validation
			cout << "Invalid entry, you must enter either \"1\" or \"2\"";
			cin.clear();
			cin.ignore(1000, '\n');
		}
		if (payoffMethod == 1) {
			sort(debts.begin(), debts.end(), [&](Debt a, Debt b) {
				return a.balance < b.balance;
				});
		}
		else if(payoffMethod == 2) {
			sort(debts.begin(), debts.end(), [&](Debt a, Debt b) {
				return a.interestRate < b.interestRate;
				});
		}
		calculatePayoff(debts, moneyForDebt);//calculates and adds the time to pay off each debt to the 2D vector

		ofstream ofile("Debt Payoff.txt");//create the output file
		writeDataFile(ofile, debts, columnLabels, moneyForDebt);
		ofile.close();
		ifstream ifile("Debt Payoff.txt");
		if (!ifile) {
			throw runtime_error("Unable to open input file");
		}
		printData(ifile);
		ifile.close();
	}

	return 0;
}

vector<Debt> getInput() {
	vector<Debt> debts;

	while (true) {
		string debtName;
		double debtBalance;
		double interestRate;
		double minimumPayment;

		cout << "Enter the name of a debt, or press Enter to quit and run the calculation: ";
		getline(cin, debtName);
		if (debtName == "") {
			break;
		}

		cout << "Enter the current debt balance: ";
		while (!(cin >> debtBalance) || debtBalance <= 0.0) {
			cout << "Error, you must enter a number greater than 0.0. Try again: ";
			cin.clear();
			cin.ignore(1000, '\n');
		}

		cout << "Enter the annual interest rate on the debt: ";
		while (!(cin >> interestRate) || interestRate < 0.0) {
			cout << "Error, you must enter a non-negative number. Try again: ";
			cin.clear();
			cin.ignore(1000, '\n');
		}

		cout << "Enter the minimum monthly payment on the debt: ";
		while (!(cin >> minimumPayment) || minimumPayment < 0.0) {
			cout << "Error, you must enter a non-negative number. Try again: ";
			cin.clear();
			cin.ignore(1000, '\n');
		}
		cin.clear();
		cin.ignore(1000, '\n');

		Debt debt{ debtName, debtBalance, interestRate, minimumPayment };
		debts.push_back(debt);
	}

	return debts;
}

double sumMinimumPayments(const vector<Debt> &debts) { //calculates the total of the minimum monthly payments
	double total = 0;
	for (const auto& debt : debts) {
		total += debt.minPayment; //adds each monthly payment to the total
	}
	return total;
}

bool debtsMaintainable(const vector<Debt>& debts, double totalActualPayments) { //calculates the total monthly payment needed to maintain all debts.
	double surplus = totalActualPayments - sumMinimumPayments(debts);
	for (const auto& debt : debts) {
		double paymentToMaintain = debt.balance * (debt.interestRate / 1200);
		if (paymentToMaintain >= debt.minPayment) {
			surplus -= paymentToMaintain - debt.minPayment;
			if (surplus < 0.0) {
				return false;
			}
		}
	}
	return true;
}

double getPayoffTime(const Debt& debt, double monthlyPmnt) {
	if (abs(debt.interestRate) < 1e-9) {
		return (debt.balance / monthlyPmnt);
	}
	double monthlyRate = debt.interestRate / 1200;
	return -(log(1 - ((debt.balance * monthlyRate) / monthlyPmnt)) / log(1 + monthlyRate));
}

double getRemainingBalance(const Debt& debt, double monthlyPmnt, double time) {
	if (abs(debt.interestRate) < 1e-9) {
		return debt.balance - (monthlyPmnt * time);
	}
	double monthlyRate = debt.interestRate / 1200;
	double growth = pow((1 + monthlyRate), time);
	return ((debt.balance * growth) - (monthlyPmnt * (growth - 1) / monthlyRate));
}

void calculatePayoff(vector<Debt>& debts, double income) {
	vector<Debt> debtReduction = debts;//A new vector that can be modified to simulate debt paydown
	double time;//Length of time to pay down each debt
	double monthlyPmnt;//the total monthly payment that will be put on each debt
	double totalTime = 0.0;//the cumulative time until each debt will be paid off
	vector<bool> paidOffEarly(debts.size(), false);//a vector for the index values of all debts that get paid off while other debts are being focused on

	for (int i = 0; i < debtReduction.size(); i++) {
		vector<int> smallIndexes;//a vector to hold the index values of small debts that will be paid off during a single "i"
		if (!paidOffEarly[i]) {//if debt i has already been paid off, the loop will skip to its next iteration
			monthlyPmnt = (income - sumMinimumPayments(debtReduction)) + debtReduction[i].minPayment;//subtracts the total monthly payments from the total money to be spent on debt, then adds back the minimum monthly payment for debt "i", resulting in the total monthly payment on debt "i"
			double rate = debtReduction[i].interestRate / 1200;//monthly interest rate
			time = getPayoffTime(debtReduction[i], monthlyPmnt);//formula for the number of months until payoff

			for (int j = i + 1; j < debtReduction.size(); j++) {//this will iterate through each remaining debt
				if (!paidOffEarly[j]) {//makes sure the debt hasn't been paid off
					double rate2 = debtReduction[j].interestRate / 1200;//the monthly interest rate of debt "j"
					double remainingBalance = getRemainingBalance(debtReduction[j], debtReduction[j].minPayment, time);

					if (remainingBalance <= 1e-9) {//if debt j will be paid off before debt "i" is done. The code from this line through line 300 all deals with this situation.
						paidOffEarly[j] = true;
						smallIndexes.push_back(j);//adds it's index to the small indexes vector
						double time2 = getPayoffTime(debtReduction[j], debtReduction[j].minPayment);//time to pay off debt j
						debts[j].payoffTime = time2 + totalTime;//adds its time to the table
						debtReduction[j].minPayment = 0.0;//sets its monthly payment to 0
					}
					else {
						debtReduction[j].balance = remainingBalance;//if it won't be paid off: update the balance of debt j
					}
				}
			}
			if (smallIndexes.size()) {//if their are any small debts that will be paid off from their minimum payments
				if (smallIndexes.size() > 1) {//if there is more than one, they will need sorted
					sort(smallIndexes.begin(), smallIndexes.end(), [&](int a, int b) {
						return debts[a].payoffTime < debts[b].payoffTime; });
				}
				double time2 = debts[smallIndexes[0]].payoffTime - totalTime;//the time paying on debt i at a specific monthly payment
				double totalJTime = 0.0;//the cumulative total time spent paying on the small debts
				for (int j = 0; j < smallIndexes.size(); j++) {
					debtReduction[i].balance = getRemainingBalance(debtReduction[i], monthlyPmnt, time2);//calculates the balance of debt i when each successive j becomes paid off
					monthlyPmnt += debtReduction[smallIndexes[j]].minPayment;//adds the minimum payment on debt j to the monthly payment for i

					totalJTime += time2;//the total time spent paying on the small debts
					if (j < smallIndexes.size() - 1) {
						time2 = (debts[smallIndexes[j + 1]].payoffTime - totalTime) - totalJTime;//the total time of the next small debt, minus the time that has already elapsed
					}
				}

				time = totalJTime + getPayoffTime(debtReduction[i], monthlyPmnt);//adds the time remaining to pay off debt i after the last small debt has been paid off from its minimum payments
			}

			totalTime += time;//add each debt's time to totalTime
			debtReduction[i].minPayment = 0.0;//sets monthly payment of debt "i" to zero
			debts[i].payoffTime = totalTime;//adds the total time until debt "i" will be paid off to the original vector
		}
	}
}

double findLongestTime(const vector<Debt>& debts) {
	double longest = 0.0;
	for (const auto& debt : debts) {
		if (debt.payoffTime > longest) {
			longest = debt.payoffTime;
		}
	}
	return longest;
}

int width(const vector<Debt>& debts, const vector<string>& nameOfColumns) {//calculates the width of the name/description column
	int maxWidth = nameOfColumns[0].size();//sets max width variable equal to the column label width
	for (const auto& d : debts) {//iterates through the vector to see if any of the names are wider than the label
		if (d.name.length() > maxWidth) {
			maxWidth = d.name.length();
		}
	}
	return maxWidth;//returns the widest value
}

void writeDataFile(ofstream& file, const vector<Debt>& debts, const vector<string>& columnLabels, double totalPayment) {
	const int NAME_WIDTH = width(debts, columnLabels) + 2;
	const int BALANCE_WIDTH = 17;
	const int INTEREST_WIDTH = 22;
	const int MIN_PAYMENT_WIDTH = 25;
	const int TIME_WIDTH = 22;

	file << "Here is your completed debt payoff plan assuming total payments of $" << totalPayment << " per month. Make the minimum payments on all debts except the first, and throw all your extra money at it until it is paid off, then continue down the list. You should be debt free in about " << ceil(findLongestTime(debts)) << " months.\n\n";

	for (int j = 0; j < columnLabels.size(); j++) {//Write the column labels to the file
		if (j == 0) {
			file << left << setw(NAME_WIDTH) << columnLabels[j];//finds the widest value in the names column, and sets the width 2 spaces wider
		}
		else {
			file << columnLabels[j] << "  ";//the rest of the column labels will be the widest element in their columns, so they only need two spaces between them
		}
	}
	file << '\n';
	for (const auto& debt : debts) {//this loop will write the table of data to the file
		file << left << setw(NAME_WIDTH) << debt.name;//write the debt names/descriptions to the file
		ostringstream ss;//stringstream object so symbols can be added to the numbers easily
		ss << "$" << fixed << setprecision(2) << debt.balance;
		file << setw(BALANCE_WIDTH) << ss.str();
		ss.clear();
		ss.str("");
		ss << debt.interestRate << "%";
		file << setw(INTEREST_WIDTH) << ss.str();
		ss.clear();
		ss.str("");
		ss << "$" << debt.minPayment;
		file << setw(MIN_PAYMENT_WIDTH) << ss.str();
		ss.clear();
		ss.str("");
		ss << setprecision(1) << debt.payoffTime;
		file << setw(TIME_WIDTH) << ss.str();
		file << '\n';
	}
}

void printData(ifstream& file) {
	cout << "\nYour debt plan has been calculated, the following information has been written to " << filesystem::current_path() / "Debt Payoff.txt" << ":\n\n";

	string s;
	while (getline(file, s)) {
		cout << s << '\n';//prints the contents of the file
	}
}