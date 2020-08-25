#pragma once
#include "sharedStructures.hpp"
#include "master.hpp"
#include "worker.hpp"

class network {
public:
	//constructor for the network object
	network() {
		//all is needed is to make and assign the mutex
		info.startTime = std::chrono::high_resolution_clock::now();
		info.endTime = 0;
		info.expectedTime = 0;
	};

	// master -> worker | sends work after appropriate delays.
	// @return :	TRUE	- Worker recieved message.
	//				FALSE	- Worker did not recieve message
	bool callWorkArg(SCH_workArg* arg) {

		workerInfo wki = workers[arg->workerId]->info;

		//determine if packet is to be dropped.
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		if (rand() % 100 < 100 * wki.reliability) {
			//packet goes through
			//wait for average latency +- %10
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));
			//send packet through
			SCH_workReply rep;
			//printf("[MASTER -> WORKER %2d] callWorkArg| Frame: %4d\n", arg->workerId, arg->frame.frameNum);
			if (workers[arg->workerId]->callWorkArg(arg, &rep)) {
				//work
				if (rep.confirmed)
				{
					std::thread th(&network::workerDoWork, this, arg->workerId);
					th.detach();
				}
				rep.workerId = arg->workerId;
				callWorkReply(&rep);
				return true;
			}
		}
		else { //printf("[DROPPED PACKET][MASTER -> WORKER %2d] callWorkArg| Frame: %4d\n", arg->workerId, arg->frame.frameNum); 
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(mtr->info.heartbeatTimeout));
		mtr->info.workers[arg->workerId].currentFrame = -1;
		return false;
	};
	// worker -> master | conformation that work has been recieved.
	// @return :	TRUE	- Master recieved message.
	//				FALSE	- Master did not recieve message
	bool callWorkReply(SCH_workReply* rep) {

		workerInfo wki = workers[rep->workerId]->info;

		//determine if packet is to be dropped.
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		if (rand() % 100 < 100 * wki.reliability) {
			//packet goes through
			//wait for average latency +- %10
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));
			//send packet through
			//printf("[WORKER %2d -> MASTER] callWorkReply  | Frame: %4d | Confirmed: %d\n", rep->workerId, rep->frameNum, rep->confirmed);
			if (mtr->callWorkReply(rep)) {
				return true;
			}
		}
		else { //printf("[DROPPED PACKET][WORKER %2d -> MASTER] callWorkReply  | Frame: %4d | Confirmed: %d\n", rep->workerId, rep->frameNum, rep->confirmed); 
		}
		return false;
	}

	// master -> worker | Heartbeat
	// @return :	TRUE	- Worker recieved message.
	//				FALSE	- Worker did not recieve message
	bool callHeartbeatArg(SCH_heartbeat* hb) {

		workerInfo wki = workers[hb->workerId]->info;

		//determine if packet is to be dropped.
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		if (rand() % 100 < 100 * wki.reliability) {
			//packet goes through
			//wait for average latency +- %10

			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));
			//send packet through
			//printf("[MASTER -> WORKER %2d] Heartbeat\n", hb->workerId);
			if (workers[hb->workerId]->callHeartbeat(hb))
			{
				callHeartbeatReply(hb);
				return true;
			}
		}
		else { //printf("[DROPPED PACKET][MASTER -> WORKER %2d] Heartbeat\n", hb->workerId); 
		}
		return false;
	}

	// worker -> master | Heartbeat
	// @return :	TRUE	- Master recieved message.
	//				FALSE	- Master did not recieve message
	bool callHeartbeatReply(SCH_heartbeat* hb) {

		workerInfo wki = workers[hb->workerId]->info;

		//determine if packet is to be dropped.
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		if (rand() % 100 < 100 * wki.reliability) {
			//packet goes through
			//wait for average latency +- %10

			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));
			//send packet through
			//printf("[WORKER %2d -> MASTER] Heartbeat\n", hb->workerId);
			if (mtr->callHeartbeat(hb))
			{
				return true;
			}
		}
		else {
			//printf("[DROPPED PACKET][WORKER %2d -> MASTER] Heartbeat\n", hb->workerId); 
		}
		return false;
	}

	// worker -> master | Existance announcement
	// @return :	TRUE	- Good communication.
	//				FALSE	- Master did not recieve message
	void callExistenceArg(SCH_existenceArg arg) {

		workerInfo wki = workers[arg.info.workerId]->info;
		bool loop = false;
		while (!loop) {
			//determine if packet is to be dropped.
			srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
			if (rand() % 100 < 100 * wki.reliability) {
				//packet goes through
				//wait for average latency +- %10
				std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));

				//send packet through
				SCH_existenceReply rep;
				//printf("[WORKER %2d -> MASTER] callExistenceArg\n", arg.info.workerId);
				if (mtr->callExistenceArg(&arg, &rep))
				{
					loop = callExistenceReply(&arg, &rep);;
				}
			}
			else { //printf("[DROPPED PACKET][WORKER %2d -> MASTER] callExistenceArg\n", arg.info.workerId); 
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 100 + 500));
		}
	}


	// master -> worker | Existence confirmation
	// @return :	TRUE	- Worker recieved message.
	//				FALSE	- Worker did not recieve message
	bool callExistenceReply(SCH_existenceArg* arg, SCH_existenceReply* rep) {

		workerInfo wki = workers[rep->workerId]->info;

		//determine if packet is to be dropped.
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		if (rand() % 100 < 100 * wki.reliability) {
			//packet goes through
			//wait for average latency +- %10
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));
			//send packet through
			//printf("[MASTER -> WORKER %2d] callExistenceReply\n", arg->info.workerId);
			return workers[rep->workerId]->callExistenceReply(rep);
		}
		else { //printf("[DROPPED PACKET][MASTER -> WORKER %2d] callExistenceReply\n", arg->info.workerId); 
		}
		return false;
	}


	// worker -> master | Existance announcement
	// @return :	TRUE	- Good communication.
	//				FALSE	- Master did not recieve message
	bool callWorkFinishedArg(SCH_workFinishedArg* arg) {

		workerInfo wki = workers[arg->workerId]->info;
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		//determine if packet is to be dropped.
		if (rand() % 100 < 100 * wki.reliability) {
			//packet goes through
			//wait for average latency +- %10
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));
			SCH_workFinishedReply reply;
			reply.workerId = arg->workerId;
			//printf("[WORKER %2d -> MASTER] callWorkFinishedArg  | Frame: %4d\n", arg->workerId, arg->frame.frameNum);
			if (mtr->callWorkFinishedArg(arg, &reply))
			{
				return callWorkFinishedReply(arg, &reply);
			}
		}
		else { //printf("[DROPPED PACKET][WORKER %2d -> MASTER] callWorkFinishedArg  | Frame: %4d\n", arg->workerId, arg->frame.frameNum); 
		}
		return false;
	}


	// master -> worker | Existence confirmation
	// @return :	TRUE	- Worker recieved message.
	//				FALSE	- Worker did not recieve message
	bool callWorkFinishedReply(SCH_workFinishedArg* arg, SCH_workFinishedReply* rep) {
		workerInfo wki = workers[rep->workerId]->info;
		srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		//determine if packet is to be dropped.
		if (rand() % 100 < 100 * wki.reliability) {
			//packet goes through
			//wait for average latency +- %10
			std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (wki.averageLatency / 10) + wki.averageLatency));
			//send packet through
			//printf("[MASTER -> WORKER %2d] callWorkFinishedReply  | Frame: %4d\n", arg->workerId, arg->frame.frameNum);
			return workers[rep->workerId]->callworkFinishedReply(rep);
		}
		else { //printf("[DROPPED PACKET][MASTER -> WORKER %2d] callWorkFinishedReply  | Frame: %4d\n", arg->workerId, arg->frame.frameNum); 
		}
		return false;
	}

	// gets current stats of simulation. 
	void getResults(networkInfo* _info) {
		_info = &info;
	};

	//delayed call which fires an event after a certain amount of time.
	void eventCall(event et) {
		//wait for event to fire.
		std::this_thread::sleep_for(std::chrono::milliseconds(et.time));
		//determine type of event
		switch (et.eType) {
		case EVENT_MASTER_DOWN:
			printf("[EVENT] Master down for %d.\n", et.downTime);
			mtr->pause();
			std::this_thread::sleep_for(std::chrono::milliseconds(et.downTime));
			mtr->start();
			break;
		case EVENT_WORKER_DOWN:
			printf("[EVENT] Worker %d down for %d.\n", et.workerid, et.downTime);
			workers[et.workerid]->pause();
			std::this_thread::sleep_for(std::chrono::milliseconds(et.downTime));
			workers[et.workerid]->start();
			break;
		case EVENT_WORKER_DIES:
			printf("[EVENT] Worker %d dead.\n", et.workerid);
			workers[et.workerid]->stop();
			break;
		default:
			break;
		}
	};

	void workerDoWork(int workerId) {

		worker* wkr = workers[workerId];
		wkr->mtx.lock();

		wkr->lastWorkStart = std::chrono::high_resolution_clock::now();
		bool loop = true;
		wkr->mtx.unlock();
		auto startTime = std::chrono::high_resolution_clock::now();
		while (loop) {

			std::this_thread::sleep_for(std::chrono::milliseconds(wkr->info.currentJob.computeTime));

			wkr->mtx.lock();

			auto currentTime = std::chrono::high_resolution_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - wkr->lastWorkStart).count() <= 0) {
				loop = false;
			}
			else {
				auto currentTime = std::chrono::high_resolution_clock::now();
				//add time to current operation
				wkr->info.currentJob.computeTime -= std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - wkr->lastWorkStart).count();
				wkr->lastWorkStart = currentTime;
			}
			wkr->mtx.unlock();
		}
		auto endTime = std::chrono::high_resolution_clock::now();


		//work finished.
		SCH_workFinishedArg arg;
		arg.frame = wkr->info.currentJob;
		arg.workerId = workerId;
		arg.frame.completed = true;
		arg.frame.computeTimeStart = startTime;
		arg.frame.computeTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
		int loops = 0;
		while (!callWorkFinishedArg(&arg)) {
			loops++;
			//printf("       Loops: %d\n", loops);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}

	//main thread which runs the master.
	//it's here for simplicity.
	void mainLoop(master* mtr) {
		//is there more work to be done
		bool loop = true;
		while (loop)
		{
			mtr->mtx.lock();
			if (mtr->info.frames.size() > 0 || mtr->info.computingFrames.size() > 0) {
				if (mtr->info.frames.size() > 0)
				{
					//are there available workers?
					auto currentTime = std::chrono::high_resolution_clock::now();
					auto it = mtr->info.frames.begin();
					for (std::pair<int, workerInfo> wk : mtr->info.workers)
					{
						if (wk.second.currentFrame < 0 &&
							std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - wk.second.lastHeartbeat).count() < mtr->info.heartbeatTimeout)
						{
							//vaild worker, assign work.
							//assign work
							if (it != mtr->info.frames.end())
							{
								SCH_workArg* arg = new SCH_workArg;
								arg->frame = *it;
								++it;
								arg->workerId = wk.second.workerId;
								mtr->info.workers[arg->workerId].currentFrame = arg->frame.frameNum;
								std::thread th(&network::callWorkArg, this, arg);
								th.detach();
							}
						}
					}
				}
				//do a heartbeat
				for (std::pair<int, workerInfo> wk : mtr->info.workers)
				{
					SCH_heartbeat* arg = new SCH_heartbeat;
					arg->workerId = wk.second.workerId;
					std::thread th(&network::callHeartbeatArg, this, arg);
					th.detach();
					auto currentTime = std::chrono::high_resolution_clock::now();
					if (std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - wk.second.lastHeartbeat).count() > mtr->info.heartbeatTimeout && wk.second.currentFrame < 0) {
						//there is a dead worker.
						mtr->info.workers[wk.second.workerId].currentFrame = -1;
						auto it = mtr->info.computingFrames.find(wk.second.workerId);
						if (it != mtr->info.computingFrames.end())
						{
							mtr->info.frames.push_back(mtr->info.computingFrames[wk.second.workerId]);
							mtr->info.computingFrames.erase(wk.second.workerId);
						}
					}
				}

			}
			else {
				//work is finished, exit loop
				loop = false;
			}
			mtr->mtx.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
		}
	}

	networkInfo info;
	master* mtr;
	std::map<int, worker*> workers;
	std::mutex mtx;
};

