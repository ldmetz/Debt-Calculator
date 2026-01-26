#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <filesystem>
#include <cstdlib>
#include <stdexcept>

struct Debt {
	std::string name{};
	double balance{};
	double interestRate{};
	double minPayment{};
	int payoffTime{ -1 };
};

std::vector<Debt> getInput();
double getTotalPayment(const std::vector<Debt>&);
int getPayoffMethod();
void sortDebts(std::vector<Debt>&, int);
void calculatePayoff(std::vector<Debt>&, double);
void writeDataFile(std::ofstream&, const std::vector<Debt>&, double);
void printData(std::ifstream&);

int main() {
	std::cout << "This program will create a personal debt payoff plan.\n";

	//Gather the debt data
	std::vector<Debt> debts; //The vector that will be used to store all debts
	debts = getInput();

	double totalPayment; //total monthly amount that will be spent to pay off debt
	try {
		totalPayment = getTotalPayment(debts);
	}
	catch(const std::runtime_error& e){
		std::cout << e.what();
		exit(0);
	}

	//Get the payoff method
	int payoffMethod;
	payoffMethod = getPayoffMethod();

	//Sort the debt vector
	sortDebts(debts, payoffMethod);

	//Calculate the payoff time for all debts
	try {
		calculatePayoff(debts, totalPayment);
	}
	catch (const std::runtime_error& e) {
		std::cout << e.what();
		exit(0);
	}

	//Create the output file
	std::ofstream ofile("Debt Payoff.txt");
	writeDataFile(ofile, debts, totalPayment);
	ofile.close();

	//Open the file
	std::ifstream ifile("Debt Payoff.txt");
	if (!ifile) {
		throw std::runtime_error("Unable to open input file");
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

std::vector<Debt> getInput() {
	std::vector<Debt> debts;

	while (true) {
		std::string debtName;
		double debtBalance;
		double interestRate;
		double minimumPayment;

		std::cout << "Enter the name of a debt, or press Enter to quit and run the calculation: ";
		std::getline(std::cin, debtName);

		if (debtName == "") {
			break;
		}

		std::cout << "Enter the current debt balance: ";
		while (!(std::cin >> debtBalance) || debtBalance <= 0.0) {
			std::cout << "Error, you must enter a number greater than 0.0. Try again: ";
			std::cin.clear();
			std::cin.ignore(1000, '\n');
		}

		std::cout << "Enter the annual interest rate on the debt: ";
		while (!(std::cin >> interestRate) || interestRate < 0.0) {
			std::cout << "Error, you must enter a non-negative number. Try again: ";
			std::cin.clear();
			std::cin.ignore(1000, '\n');
		}

		std::cout << "Enter the minimum monthly payment on the debt: ";
		while (!(std::cin >> minimumPayment) || minimumPayment < 0.0) {
			std::cout << "Error, you must enter a non-negative number. Try again: ";
			std::cin.clear();
			std::cin.ignore(1000, '\n');
		}

		std::cin.clear();
		std::cin.ignore(1000, '\n');

		Debt debt{ debtName, debtBalance, interestRate, minimumPayment };
		debts.push_back(debt);
	}

	return debts;
}

/**
* Get the total amount the user plans to spend on debt and ensure it is enough to meet minimum payments
* @return the total monthly payment on debt
*/

double getTotalPayment(const std::vector<Debt>& debts) {
	double totalPayment;
	std::cout << "Enter the total monthly amount you can spend on debt payments: ";
	while (!(std::cin >> totalPayment)) {
		std::cout << "Incorrect input. You must enter a number (no $ sign): ";
		std::cin.clear();
		std::cin.ignore(1000, '\n');
	}

	double sumMinimumPayments{ 0.0 };
	for (const auto& debt : debts) {
		sumMinimumPayments += debt.minPayment;
	}

	//Check if the user has enough money to meet the minimum payments
	if (totalPayment < sumMinimumPayments) {
		throw std::runtime_error("You do not have enough disposable income to meet your minimum payments.");
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
	std::cout << "\nDebt payoff methods:\n"
		<< "1. Debt Snowball (smallest debt first)\n"
		<< "2. Debt Avalanche (highest interest rate first)\n"
		<< "Enter your selection : ";

	while (!(std::cin >> payoffMethod) && (payoffMethod != 1 && payoffMethod != 2)) {
		std::cout << "Invalid entry, you must enter either \"1\" or \"2\": ";
		std::cin.clear();
		std::cin.ignore(1000, '\n');
	}

	return payoffMethod;
}

/**
* Sorts the debts according to the selected payoff method
*/

void sortDebts(std::vector<Debt>& debts, int payoffMethod) {
	//If debt snowball was selected, sort by ascending balance
	if (payoffMethod == 1) {
		std::sort(debts.begin(), debts.end(), [](const Debt& a, const Debt& b) {
			return a.balance < b.balance;
			});
	}

	//If debt avalanche was selected, sort be descending interest rate
	else if (payoffMethod == 2) {
		std::sort(debts.begin(), debts.end(), [](const Debt& a, const Debt& b) {
			return a.interestRate > b.interestRate;
			});
	}
}

void calculatePayoff(std::vector<Debt>& debts, double totalPayment) {
	int months{ 0 };
	std::vector<Debt> debtPayoff(debts); //A vector to simulate debt paydown without modifying the original balances
	int target = 0; //The index value of the debt to target with all extra funds
	bool allPaidOff{ false };

	while (!allPaidOff) {
		months++;
		double totalAvailable{ totalPayment };

		//Add interest and make minimum payments on all debts
		for (auto& debt : debtPayoff) {
			if (debt.balance <= 1e-9) {
				continue;
			}
			debt.balance += debt.balance * (debt.interestRate / 1200.0);
			double payment = std::min(debt.balance, debt.minPayment);
			totalAvailable -= payment;
			debt.balance -= payment;
		}

		//Apply the remaining available money towards the other debts
		for (int i = target; i < debtPayoff.size(); i++) {
			Debt& targetDebt{ debtPayoff[i] };

			if (targetDebt.balance <= 1e-9) {
				continue;
			}

			double payment = std::min(targetDebt.balance, totalAvailable);
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
			throw std::runtime_error("Unsustainable debt load. Your debt will take more than 50 years to pay off.");
		}
	}
}

/**
* Finds the debt that takes the longest time to pay off
* @return the number of months to pay off all debts
*/

int findLongestTime(const std::vector<Debt>& debts) {
	int longest{ 0 };
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

int width(const std::vector<Debt>& debts, const std::vector<std::string>& nameOfColumns) {
	int maxWidth{ nameOfColumns[0].size() };

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

void writeDataFile(std::ofstream& file, const std::vector<Debt>& debts, double totalPayment) {
	const std::vector<std::string> columnLabels { "Name/description", "Current balance", "Annual Interest Rate", "Minimum Monthly Payment", "Months until Payoff" };

	const int NAME_WIDTH{ width(debts, columnLabels) + 2 };
	const int BALANCE_WIDTH{ 17 };
	const int INTEREST_WIDTH{ 22 };
	const int MIN_PAYMENT_WIDTH{ 25 };
	const int TIME_WIDTH{ 22 };

	file << "Here is your completed debt payoff plan assuming total payments of $" << totalPayment << " per month. Make the minimum payments on all debts except the first, and throw all your extra money at it until it is paid off, then continue down the list. You should be debt free in about " << findLongestTime(debts) << " months.\n\n";

	//Write the column labels to the file
	for (int j = 0; j < columnLabels.size(); j++) {
		if (j == 0) {
			file << std::left << std::setw(NAME_WIDTH) << columnLabels[j];
		}
		else {
			file << columnLabels[j] << "  ";
		}
	}
	file << '\n';

	//Write the debt data to the file
	for (const auto& debt : debts) {
		file << std::left << std::setw(NAME_WIDTH) << debt.name;

		std::ostringstream ss;
		ss << "$" << std::fixed << std::setprecision(2) << debt.balance;
		file << std::setw(BALANCE_WIDTH) << ss.str();
		ss.clear();
		ss.str("");

		ss << debt.interestRate << "%";
		file << std::setw(INTEREST_WIDTH) << ss.str();
		ss.clear();
		ss.str("");

		ss << "$" << debt.minPayment;
		file << std::setw(MIN_PAYMENT_WIDTH) << ss.str();
		ss.clear();
		ss.str("");

		ss << debt.payoffTime;
		file << std::setw(TIME_WIDTH) << ss.str();
		file << '\n';
	}
}

/**
* Prints the data from the file
*/

void printData(std::ifstream& file) {
	std::cout << "\nYour debt plan has been calculated, the following information has been written to "
		<< std::filesystem::current_path() / "Debt Payoff.txt" << ":\n\n";

	std::string s;
	while (std::getline(file, s)) {
		std::cout << s << '\n';
	}
}