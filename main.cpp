// CSC462Project.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>

#include "sharedStructures.hpp"
#include "network.hpp"
#include "master.hpp"
#include "worker.hpp"

void startTest(				// runs a simulation of the master-worker system.
	int numWorkers,			// number of workers
	int minLatency,		// minimum latency per worker in ms.
	int maxLatency,		// maximum latency per worker in ms.
	float dropPacketChance, // chance between 0 and 1 that the connection will drop a packet.
	int masterDownEvents,	// number of times the master will go down.
	int masterDownTime,	// amount of time the master will spend down.
	int workerDownEvents,	// number of times workers will go down.
	int workerDownTime,	// amount of time the workers will stay down.
	int deadWorkerEvents,		// number of workers that will die sometime during the run.
	workload work,			// work to be done.
	network *nt
) {
	srand(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
	//determine info object
	//expected time is best possible time in ms.
	nt->info.expectedTime = work.numFrames * work.workPerFrame;

	//generate all objects.
	//generate workers
	int latencyVariation = maxLatency - minLatency;
	for (int i = 0; i < numWorkers; i++) {
		worker* wkr = (new worker);
		wkr->info.averageLatency = (rand() % latencyVariation) + minLatency;
		wkr->info.reliability = dropPacketChance;
		wkr->info.workerId = i;
		wkr->info.currentFrame = -1;
		wkr->info.killed = false;
		wkr->info.paused = false;
		
		nt->workers[i] = wkr;
	}

	//generate master
	masterInfo mtrInf;
	mtrInf.work = work;
	mtrInf.paused = false;
	mtrInf.heartbeatTimeout = 5000; // 5 seconds
	master mtrr(mtrInf);
	nt->mtr = &mtrr;
	nt->mtr->generateFrames();

	//generate events
	std::vector<event> eventList;
	//master down events
	for (int i = 0; i < masterDownEvents; i++) {
		event et;
		et.eType = EVENT_MASTER_DOWN;
		et.downTime = masterDownTime;
		//event will happen within the expected best time
		et.time = rand() % nt->info.expectedTime;
		eventList.push_back(et);
	}

	//worker down events
	for (int i = 0; i < workerDownEvents; i++) {
		event et;
		et.eType = EVENT_WORKER_DOWN;
		et.downTime = workerDownTime;
		//event will happen within the expected best time
		et.time = rand() % nt->info.expectedTime;
		et.workerid = rand() % numWorkers;
		eventList.push_back(et);
	}

	//worker killed events
	for (int i = 0; i < deadWorkerEvents; i++) {
		event et;
		et.eType = EVENT_WORKER_DIES;
		et.downTime = workerDownTime;
		//event will happen within the expected best time
		et.time = rand() % nt->info.expectedTime;
		et.workerid = i;
		if (i < numWorkers) // can't kill more workers than exist.
		{
			eventList.push_back(et);
		}
	}

	//start event threads
	for (event et : eventList)
	{
		std::thread th(&network::eventCall,nt, et);
		th.detach();
	}
	nt->info.startTime = std::chrono::high_resolution_clock::now();
	//start master
	std::thread mainThread(&network::mainLoop, nt, nt->mtr);
	//printf("MainThreadCalled\n");
	
	//wait for half a second
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::vector<std::future<void>> futures;
	//wake up workers
	for (std::pair<int, worker*> wk : nt->workers)
	{
		SCH_existenceArg arg;
		arg.info = wk.second->info;
		//std::thread th(&network::callExistenceArg, this, &arg);
		std::thread th(&network::callExistenceArg,nt,  arg);
		th.detach();
	}

	

	//wait for all work to be done
	mainThread.join();
	auto currentTime = std::chrono::high_resolution_clock::now();
	nt->info.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - nt->info.startTime).count();
};

int main()
{
	network net;
	workload work;
	work.numFrames = 50;
	work.workPerFrame = 1000;
	work.workVariance = 100;
	startTest(20, 100, 200, 0.95f, 0, 0, 0, 0, 0, work, &net);
	printf("Final Results: [%d] miliseconds\n", net.info.endTime);
}
