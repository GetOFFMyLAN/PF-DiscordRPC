// ConsoleApplicationRPC.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "DiscordRPC.h"

using std::cout;
using std::endl;
using std::cin;

void printWelcome() {
	cout << "    ____  __                 __  _______ __                      " << endl;
	cout << "   / __ \\/ /___ _____  ___  / /_/ ____(_) /_____  ___  __________" << endl;
	cout << "  / /_/ / / __ `/ __ \\/ _ \\/ __/ /_  / / __/ __ \\/ _ \\/ ___/ ___/" << endl;
	cout << " / ____/ / /_/ / / / /  __/ /_/ __/ / / /_/ / / /  __(__  |__  ) " << endl;
	cout << "/_/   /_/\\__,_/_/ /_/\\___/\\__/_/   /_/\\__/_/ /_/\\___/____/____/  \n" << endl;
	cout << "made by vain (https://vain.pw)" << endl;

	// get time and display it
	char buf[26];
	struct tm clk;
	time_t currTime;
	time(&currTime);
	localtime_s(&clk, &currTime);
	asctime_s(buf, &clk);
	cout << "Local Time: " << buf << "\n" << endl;

}

void printInstructions() {
	cout << "Welcome to Planet fitness! \n\t - If you would like to"
		<< " start your workout, type 'start'\n";
	cout << "\t - If you would like to make changes to your routine,"
		<< " type 'edit'\n" << endl;
}


// TODO: add input options for more user control (use tolower and strings??)
int getChoice(const char input[]) {
	if (input == "start") return 1;
	else if (input == "edit") return 2;
	return 0;
}

int main()
{
	DiscordRPC app;
	printWelcome();
	printInstructions();
	app.run();
	cout << "An update failed so the app is closing" << endl;
}
