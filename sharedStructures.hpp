#pragma once
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>
#include <random>
#include <memory>
#include <iostream>
#include <future>


//structs
struct workload {
	int numFrames;
	int workPerFrame;
	int workVariance;
};

struct workloadFrame {
	int frameNum;
	int computeTime;
	std::chrono::steady_clock::time_point computeTimeStart;
	bool completed;
};

struct workerInfo {
	int workerId;
	int currentFrame;
	int averageLatency;
	float reliability;
	workloadFrame currentJob;
	bool killed;
	bool paused;
	std::chrono::steady_clock::time_point lastHeartbeat;
};

struct masterInfo {
	workload work;
	std::map<int, workerInfo> workers;
	std::vector<workloadFrame> frames;
	std::map<int, workloadFrame> computingFrames;
	std::vector<workloadFrame> completedFrames;

	bool paused;
	int heartbeatTimeout;	//amount of time that passes before the master assumes the worker is dead.
};

enum eventType {
	EVENT_MASTER_DOWN,
	EVENT_WORKER_DOWN,
	EVENT_WORKER_DIES
};

struct event {
	eventType eType;
	int time;
	int downTime;
	int workerid;
};

struct networkInfo {
	std::chrono::steady_clock::time_point startTime;
	int expectedTime;
	int endTime;
};

//Schema

struct SCH_heartbeat {
	int workerId;
};

///// Master -> Worker

struct SCH_workArg {
	int workerId;
	workloadFrame frame;
};

struct SCH_existenceReply {
	int workerId;
};

struct SCH_workFinishedReply {
	int workerId;
};

///// Worker -> Master

struct SCH_workReply {
	bool confirmed;
	int workerId;
	int frameNum;
};

struct SCH_existenceArg {
	workerInfo info;
};

struct SCH_workFinishedArg {
	int workerId;
	workloadFrame frame;
};




//Objects



//Functions
