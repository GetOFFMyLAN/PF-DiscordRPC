#pragma once
#include <iostream>
#include <thread>
#include <csignal>
#include <time.h>
#include <string>
#include "../discord-files/discord.h"
#include "WoutContainer.h"
#include "hidden.h"

#define DEBUG 0

namespace {
	volatile bool interrupted{ false };
}

enum class DiscordRPC_CBFlags {
	UserFound,
	InvalidUser,
	UpdateSuccess,
	UpdateFail,
	ErrorState,
	Ok
};

struct DiscordState {
	std::unique_ptr<discord::Core> core;
	discord::User currUser;
	discord::Activity currActivity;
	DiscordRPC_CBFlags status;

	DiscordState& operator=(DiscordState& rhs) {
		if (this != &rhs) {
			this->core.reset(rhs.core.get());
			this->currUser = rhs.currUser;
			this->currActivity = rhs.currActivity;
		}
		return *this;
	}
};

class DiscordRPC {
private:
	DiscordState state{};
	workout::Container wouts;

	// helper methods
	void update(size_t, unsigned int, time_t);
	void init(DiscordState&); // instantiates core, user properties

public:
	DiscordRPC();
	void run();	 // performs callbacks while not interupted by signal
};

