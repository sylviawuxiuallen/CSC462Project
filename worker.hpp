#pragma once
#include "sharedStructures.hpp"

class worker {
public:
	worker() {

	};
	worker(workerInfo _info) {
		info = _info;
	};
	//RPC calls

	//gotten call to work.
	bool callWorkArg(SCH_workArg* arg, SCH_workReply* rep) {
		mtx.lock();
		
		bool ret = false;
		if (!info.paused)
		{
			if (info.currentFrame < 0)
			{
				
				info.currentJob = arg->frame;
				info.currentFrame = info.currentJob.frameNum;
				rep->confirmed = true;
				rep->frameNum = info.currentFrame;
				rep->workerId = info.workerId;
				//printf("     [WORKER %2d] Got work, %d ACCEPTED\n", info.workerId, info.currentFrame);
			}
			else {
				//printf("     [WORKER %2d] Got work, %d REJECTED\n", info.workerId, arg->frame.frameNum);
				rep->confirmed = false;
				rep->frameNum = info.currentFrame;
				rep->workerId = info.workerId;
			}
			
			ret = true;
		}
		mtx.unlock();
		return ret;
	}

	//master recieved work
	bool callworkFinishedReply(SCH_workFinishedReply* rep) {
		mtx.lock();
		bool ret = false;
		if (!info.paused)
		{
			rep->workerId = info.workerId;
			info.currentFrame = -1;
			ret = true;
		}
		mtx.unlock();
		return ret;
	}

	//gotten heartbeat
	bool callHeartbeat(SCH_heartbeat* hb) {
		mtx.lock();
		bool ret = false;
		if (!info.paused)
		{
			info.lastHeartbeat = std::chrono::high_resolution_clock::now();
			ret = true;
		}
		mtx.unlock();
		return ret;
	}

	//master knows I exist
	bool callExistenceReply(SCH_existenceReply* rep) {
		mtx.lock();
		bool ret = false;
		if (!info.paused)
		{
			info.lastHeartbeat = std::chrono::high_resolution_clock::now();
			ret = true;
		}
		mtx.unlock();
		return ret;
	}



	//network management calls

	//worker unpauses and restores previous state.
	void start() {
		mtx.lock();
		if (!info.killed && info.paused) {
			info.paused = false;
			lastWorkStart = std::chrono::high_resolution_clock::now();
			//add time to current operation
			info.currentJob.computeTime += std::chrono::duration_cast<std::chrono::milliseconds>(lastWorkStart - pauseStart).count();
		}
		mtx.unlock();
	}
	//worker pauses and stores current state.
	void pause() {
		mtx.lock();

		if (!info.paused)
		{
			info.paused = true;
			//get current work status.
			pauseStart = std::chrono::high_resolution_clock::now();
			//add time to current operation
			info.currentJob.computeTime -= std::chrono::duration_cast<std::chrono::milliseconds>(pauseStart - lastWorkStart).count();
		}

		mtx.unlock();
	}
	void stop() {
		mtx.lock();

		info.paused = true;
		info.killed = true;

		mtx.unlock();
	}

	workerInfo info;

	bool doWork() {

	}

	std::chrono::steady_clock::time_point lastWorkStart;
	std::chrono::steady_clock::time_point pauseStart;

	std::mutex mtx;
};