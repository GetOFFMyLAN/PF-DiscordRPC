#include "DiscordRPC.h"

using std::cout;
using std::endl;
using std::string;

DiscordRPC::DiscordRPC() : state{ NULL } {
}

void DiscordRPC::update(size_t idx, unsigned int rep, time_t start) {
	discord::Activity act {};
	act.SetState(wouts.activities.at(idx).c_str());
	act.SetDetails(wouts.places.at(idx).c_str());
	act.SetType(discord::ActivityType::Playing);
	act.SetInstance(true);
	act.GetTimestamps().SetStart(start);
	act.GetTimestamps().SetEnd(start + wouts.totalTime.at(idx));
	act.GetParty().GetSize().SetCurrentSize(rep);
	act.GetParty().GetSize().SetMaxSize(wouts.maxReps.at(idx));
	act.GetAssets().SetLargeImage("logo");
	act.GetAssets().SetLargeText("LUNK ALARM GOING OFF");
	DiscordRPC_CBFlags status;
	this->state.core.get()->ActivityManager().UpdateActivity(act, [&status](discord::Result result) {
			status = ((result == discord::Result::Ok) ? DiscordRPC_CBFlags::UpdateSuccess : DiscordRPC_CBFlags::UpdateFail);
		});
	this->state.status = status;
	this->state.currActivity = act;
}

void DiscordRPC::init(DiscordState& inst) {
	// load workouts from cfg file in include folder
	try {
		this->wouts.ldData(WOUT_FILE);
	}
	catch (std::invalid_argument& err) {
		cout << err.what() << endl;
	}
	catch (std::exception& err) {
		cout << err.what() << endl;
	}

	// instantiate core
	discord::Core* core{};
	auto result = discord::Core::Create(APP_ID, DiscordCreateFlags_Default, &core);
	inst.core.reset(core);
	if (!inst.core || result != discord::Result::Ok) {
		cout << "failed to initialize RPC" << endl;
		//exit(1); TODO: throw error here instead
	}

	// set log hook for core and run callbacks
	core->SetLogHook(discord::LogLevel::Debug, [](discord::LogLevel level, const char* msg) {
		cout << "Discord: " << static_cast<uint32_t>(level) << " - " << msg << endl;
	});
	
	// get current user
	inst.core->UserManager().OnCurrentUserUpdate.Connect([&inst]() {
		inst.core->UserManager().GetCurrentUser(&inst.currUser);
		if (inst.currUser.GetId() != 0) {
			inst.status = DiscordRPC_CBFlags::UserFound;
			std::cout << "Enjoy your workout " << inst.currUser.GetUsername() << "#"
				<< inst.currUser.GetDiscriminator() << "!\n";
		}
		else {
			inst.status = DiscordRPC_CBFlags::InvalidUser;
		}
	});

	if (inst.status != DiscordRPC_CBFlags::UserFound) {
		throw std::runtime_error("User not found on startup/update");
	}

	// create an initial default activity in case our
	// updater fails to begin the workout sequence
	discord::Activity act{};
	act.SetState("Drinking Protein Shake");
	act.SetDetails("Entering the Gym");
	act.SetType(discord::ActivityType::Playing);
	act.SetInstance(true);
	act.GetAssets().SetLargeImage("logo");
	act.GetAssets().SetLargeText("LUNK ALARM ABOUT TO GO OFF");
	inst.core.get()->ActivityManager().UpdateActivity(act, [&inst](discord::Result result) {
			if (result == discord::Result::Ok) {
				inst.status = DiscordRPC_CBFlags::UpdateSuccess;
				cout << "Starting workout routine..." << endl;
			}
			else {
				inst.status = DiscordRPC_CBFlags::UpdateFail;
			}
		});
	inst.currActivity = act;
}

void DiscordRPC::run() {
	try {
		this->init(this->state);
	}
	catch (std::runtime_error& err) {
		cout << "Failed to get user. Exiting program" << endl;
		std::this_thread::sleep_for(std::chrono::seconds(2));
		exit(1);
	}
	std::signal(SIGINT, [](int) { interrupted = true; });
	
	size_t idx = 0;
	int repCnt = 1;
	time_t start;
	time_t currTime;
	time(&start);
	this->update(idx, repCnt, start); // give initial update from default

	do {
		/*if (!(this->state.status == DiscordRPC_CBFlags::UpdateSuccess || this->state.status == DiscordRPC_CBFlags::Ok)) {
			interrupted = true;
		}*/

		time(&currTime);
		// go to next workout if we have exceeded the workout time
		if (difftime(currTime, start) >= wouts.totalTime.at(idx)) {
			cout << "finished set of " << wouts.activities.at(idx) << endl;
			idx = (idx + 1) % this->wouts.size;
			repCnt = 1;
			start = currTime;
			this->update(idx, repCnt, start);
			// wait 1 second so we don't continually update activity the continue execution
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			continue;
		}

		int cutoff = (wouts.totalTime.at(idx) / wouts.maxReps.at(idx));
		
		// increase rep count if we hit the cutoff point
		if (static_cast<int>(difftime(currTime, start)) % cutoff >= cutoff - 1) {
			// wait 1 second so we don't continually update activity the continue execution
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			repCnt++;
			this->update(idx, repCnt, start);
		}

		this->state.core->RunCallbacks();
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	} while (!interrupted);
}