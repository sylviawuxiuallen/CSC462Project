#pragma once
#include "sharedStructures.hpp"

class master {
public:
	master() {

	};
	master(masterInfo _info) {
		info = _info;
	};

	//RPC calls
	//worker confirming work
	bool callWorkReply(SCH_workReply* rep) {
		//add work to list.
		mtx.lock();
		bool ret = false;
		if (!info.paused)
		{
			if (rep->confirmed) {
				//find frame in list of frames.
				std::vector<workloadFrame>::iterator it = std::find_if(info.frames.begin(), info.frames.end(),
					[&rep](workloadFrame val) {
						if (rep->frameNum == val.frameNum) return true;
						return false;
					});
				if (it != info.frames.end())
				{
					//found frame
					//add frame to currently computing frames.
					info.computingFrames[rep->workerId] = *it;
					//remove frames from work queue.
					info.frames.erase(it);
					ret = true;
					info.workers[rep->workerId].currentFrame = rep->frameNum;
				}
			}
			else {
				//worker is working on some other frame.
				info.workers[rep->workerId].currentFrame = rep->frameNum;
			}
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
			info.workers[hb->workerId].lastHeartbeat = std::chrono::high_resolution_clock::now();
			ret = true;
		}
		mtx.unlock();
		return ret;
	}

	bool callWorkFinishedArg(SCH_workFinishedArg* arg, SCH_workFinishedReply* reply) {
		mtx.lock();
		bool ret = false;
		if (!info.paused)
		{

			//find frame
			try {
				info.computingFrames.erase(arg->workerId);
				//frame is in map
				std::vector<workloadFrame>::iterator it = std::find_if(info.completedFrames.begin(), info.completedFrames.end(),
					[&arg](workloadFrame val) {
						if (arg->frame.frameNum == val.frameNum) return true;
						return false;
					});
				if (it == info.completedFrames.end())
				{
					info.completedFrames.push_back(arg->frame);
				}
				info.workers[arg->workerId].currentFrame = -1;
				reply->workerId = arg->workerId;
			}
			catch (std::out_of_range& e) {
				//frame not in map.
			}
			ret = true;
		}
		mtx.unlock();
		return ret;
	}
	

	//worker announcing presence
	bool callExistenceArg(SCH_existenceArg *arg, SCH_existenceReply* rep) {
		mtx.lock();
		bool ret = false;
		if (!info.paused)
		{
			//add worker to list of workers.
			info.workers[arg->info.workerId] = arg->info;
			ret = true;
			rep->workerId = arg->info.workerId;
		}
		mtx.unlock();
		return ret;
	}

	//Network management calls

	//unpauses master
	void start() {
		mtx.lock();
		info.paused = false;
		mtx.unlock();
	}			

	//pauses master, state is resumed when master starts again.
	void pause() {
		mtx.lock();
		info.paused = true;
		mtx.unlock();
	}

	void generateFrames() {
		mtx.lock();
		//printf("Generating Frames\n");
		for (int i = 0; i < info.work.numFrames; i++)
		{
			workloadFrame fr;
			fr.computeTime = info.work.workPerFrame + (rand() % info.work.workVariance);
			fr.completed = false;
			fr.frameNum = i;
			info.frames.push_back(fr);
		}
		mtx.unlock();
	}

	masterInfo info;
	std::mutex mtx;
};