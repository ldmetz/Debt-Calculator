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
	int payoffTime { -1 };
};

vector<Debt> getInput();
double getTotalPayment(const vector<Debt>&);
int getPayoffMethod();
void sortDebts(vector<Debt>&, int);
void calculatePayoff(vector<Debt>&, double);
void writeDataFile(ofstream&, const vector<Debt>&, double);
void printData(ifstream&);

int main() {
	vector<Debt> debts; //The vector that will be used to store all debts
	double totalPayment; //total monthly amount that will be spent to pay off debt
	int payoffMethod;

	//Gather the debt data
	cout << "This program will create a personal debt payoff plan.\n";
	debts = getInput();

	try {
		totalPayment = getTotalPayment(debts);
	}
	catch(const runtime_error& e){
		cout << e.what();
		exit(0);
	}

	//Get the payoff method
	payoffMethod = getPayoffMethod();

	//Sort the debt vector
	sortDebts(debts, payoffMethod);

	//Calculate the payoff time for all debts
	try {
		calculatePayoff(debts, totalPayment);
	}
	catch (const runtime_error& e) {
		cout << e.what();
		exit(0);
	}

	//Create the output file
	ofstream ofile("Debt Payoff.txt");
	writeDataFile(ofile, debts, totalPayment);
	ofile.close();

	//Open the file
	ifstream ifile("Debt Payoff.txt");
	if (!ifile) {
		throw runtime_error("Unable to open input file");
	}

	//Print the data to the console
	printData(ifile);
	ifile.close();

	return 0;
}

/**
* Gathers a list of Debts from user input
* @return a vector of Debt objects
*/

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

/**
* Get the total amount the user plans to spend on debt and ensure it is enough to meet minimum payments
* @return the total monthly payment on debt
*/

double getTotalPayment(const vector<Debt>& debts) {
	double totalPayment;
	cout << "Enter the total monthly amount you can spend on debt payments: ";
	while (!(cin >> totalPayment)) {
		cout << "Incorrect input. You must enter a number (no $ sign): ";
		cin.clear();
		cin.ignore(1000, '\n');
	}

	double sumMinimumPayments = 0.0;
	for (const auto& debt : debts) {
		sumMinimumPayments += debt.minPayment;
	}

	//Check if the user has enough money to meet the minimum payments
	if (totalPayment < sumMinimumPayments) {
		throw runtime_error("You do not have enough disposable income to meet your minimum payments.");
	}

	return totalPayment;
}

/**
* Get the payoff method
* @return the user's chosen payoff method
*/

int getPayoffMethod() {
	int payoffMethod;

	//Show the menu of debt payoff options
	cout << "\nDebt payoff methods:\n1. Debt Snowball (smallest debt first)\n2. Debt Avalanche (highest interest rate first)\nEnter your selection: ";
	while (!(cin >> payoffMethod) && (payoffMethod != 1 && payoffMethod != 2)) {
		cout << "Invalid entry, you must enter either \"1\" or \"2\": ";
		cin.clear();
		cin.ignore(1000, '\n');
	}

	return payoffMethod;
}

/**
* Sorts the debts according to the selected payoff method
*/

void sortDebts(vector<Debt>& debts, int payoffMethod) {
	//If debt snowball was selected, sort by ascending balance
	if (payoffMethod == 1) {
		sort(debts.begin(), debts.end(), [](const Debt& a, const Debt& b) {
			return a.balance < b.balance;
			});
	}

	//If debt avalanche was selected, sort be descending interest rate
	else if (payoffMethod == 2) {
		sort(debts.begin(), debts.end(), [](const Debt& a, const Debt& b) {
			return a.interestRate > b.interestRate;
			});
	}
}

void calculatePayoff(vector<Debt>& debts, double totalPayment) {
	int months = 0;
	vector<Debt> debtPayoff(debts); //A vector to simulate debt paydown without modifying the original balances
	int target = 0; //The index value of the debt to target with all extra funds
	bool allPaidOff = false;

	while (!allPaidOff) {
		months++;
		double totalAvailable = totalPayment;

		//Add interest and make minimum payments on all debts
		for (auto& debt : debtPayoff) {
			if (debt.balance <= 1e-9) {
				continue;
			}
			debt.balance += debt.balance * (debt.interestRate / 1200.0);
			double payment = min(debt.balance, debt.minPayment);
			totalAvailable -= payment;
			debt.balance -= payment;
		}

		//Apply the remaining available money towards the other debts
		for (int i = target; i < debtPayoff.size(); i++) {
			Debt& targetDebt = debtPayoff[i];

			if (targetDebt.balance <= 1e-9) {
				continue;
			}

			double payment = min(targetDebt.balance, totalAvailable);
			totalAvailable -= payment;
			targetDebt.balance -= payment;

			if (abs(totalAvailable) <= 1e-9) {
				break;
			}
		}

		//Set the payoff time for any debts that have been paid off this month
		for (int i = 0; i < debtPayoff.size(); i++) {
			if (debts[i].payoffTime == -1 && abs(debtPayoff[i].balance) <= 1e-9) {
				debts[i].payoffTime = months;
			}
		}
		
		//Move the target debt if it has been paid off
		while (target < debtPayoff.size() && debts[target].payoffTime != -1) {
			target++;
		}

		//If the target has moved past the end of the debt list, all debts have been paid off
		if (target == debtPayoff.size()) {
			allPaidOff = true;
		}

		//If the debt will take an unreasonable time to pay off at the current levels
		if (months > 600) {
			throw runtime_error("Unsustainable debt load. Your debt will take more than 50 years to pay off.");
		}
	}
}

/**
* Finds the debt that takes the longest time to pay off
* @return the number of months to pay off all debts
*/

int findLongestTime(const vector<Debt>& debts) {
	int longest = 0;
	for (const auto& debt : debts) {
		if (debt.payoffTime > longest) {
			longest = debt.payoffTime;
		}
	}
	return longest;
}

/**
* Calculates the width of the debt name/description column
* @return the width of the widest element in the column
*/

int width(const vector<Debt>& debts, const vector<string>& nameOfColumns) {
	int maxWidth = nameOfColumns[0].size();

	//See if there are any debt names that are wider than the column label
	for (const auto& d : debts) {
		if (d.name.length() > maxWidth) {
			maxWidth = d.name.length();
		}
	}
	return maxWidth;
}

/**
* Writes the debt payoff timeline to a text file
*/

void writeDataFile(ofstream& file, const vector<Debt>& debts, double totalPayment) {
	const vector<string> columnLabels = { "Name/description", "Current balance", "Annual Interest Rate", "Minimum Monthly Payment", "Months until Payoff" };

	const int NAME_WIDTH = width(debts, columnLabels) + 2;
	const int BALANCE_WIDTH = 17;
	const int INTEREST_WIDTH = 22;
	const int MIN_PAYMENT_WIDTH = 25;
	const int TIME_WIDTH = 22;

	file << "Here is your completed debt payoff plan assuming total payments of $" << totalPayment << " per month. Make the minimum payments on all debts except the first, and throw all your extra money at it until it is paid off, then continue down the list. You should be debt free in about " << findLongestTime(debts) << " months.\n\n";

	//Write the column labels to the file
	for (int j = 0; j < columnLabels.size(); j++) {
		if (j == 0) {
			file << left << setw(NAME_WIDTH) << columnLabels[j];
		}
		else {
			file << columnLabels[j] << "  ";
		}
	}
	file << '\n';

	//Write the debt data to the file
	for (const auto& debt : debts) {
		file << left << setw(NAME_WIDTH) << debt.name;

		ostringstream ss;
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

		ss << debt.payoffTime;
		file << setw(TIME_WIDTH) << ss.str();
		file << '\n';
	}
}

/**
* Prints the data from the file
*/

void printData(ifstream& file) {
	cout << "\nYour debt plan has been calculated, the following information has been written to " << filesystem::current_path() / "Debt Payoff.txt" << ":\n\n";

	string s;
	while (getline(file, s)) {
		cout << s << '\n';
	}
}