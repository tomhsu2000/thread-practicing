/*
模擬RMS或EDF排程演算法
在本作業中,利用模擬的方式來熟悉 Real-time Process Scheduling 的觀念
編譯: gcc -o s1071533_prog4 s1071533_prog4.cpp -lstdc++
執行: ./s1071533_prog4 p 1082-prog4-data.txt
*/
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <queue>
using namespace std;

struct Task
{
	int tid, R, C, D, T;
	int remaining_C;// the remaining CUP brust
	bool working;// distinguish whether is the current working task
	Task()
		: tid(0), R(0), C(0), D(0), T(0), remaining_C(0), working(false){};
	Task(int tid, int R, int C, int D, int T)
		: tid(tid), R(R), C(C), D(D), T(T), remaining_C(C), working(false){};
};

// compartor of the arrival time
struct compareR
{
	bool operator()(Task const &t1, Task const &t2)
	{
		return t1.R > t2.R;
	}
};

// compartor of the period
struct compareT
{
	bool operator()(Task const &t1, Task const &t2)
	{
		return t1.T > t2.T;
	}
};

// compartor of the deadline which is the arrival time add the deadline preiod
struct compareD
{
	bool operator()(Task const &t1, Task const &t2)
	{
		int t1_deadline = t1.R + t1.D, t2_deadline = t2.R + t2.D;
		if (t1_deadline == t2_deadline)
		{
			if (t1.working == t2.working)
				return t1.tid > t2.tid;
			else
				return t1.working < t2.working;
		}
		return t1_deadline > t2_deadline;
	}
};
int simu_time;
priority_queue<Task, vector<Task>, compareR> arrival_que;

void error_and_die(const char *msg);

void readFile(char file_name[]);

template <class T>
void simulate(T &task_que);

int main(int argc, char *argv[])
{
	int mode = (int)atoi(argv[1]);
	priority_queue<Task, vector<Task>, compareT> RMS;
	priority_queue<Task, vector<Task>, compareD> EDF;
	readFile(argv[2]);
	if (mode == 0)
		simulate(RMS);
	else
		simulate(EDF);
}

void error_and_die(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

void readFile(char file_name[])
{
	char temp[50];
	fstream readFile(file_name, ios::in);
	if (!readFile)
		error_and_die("File");
	while (readFile.getline(temp, 50))
	{
		if (temp[0] != '#')
		{
			simu_time = (int)atoi(temp);
			break;
		}
	}
	while (readFile.getline(temp, 50))
	{
		if (temp[0] != '#')
		{
			Task task;
			task.tid = (int)atoi(temp);
			task.R = (int)atoi(temp + 2);
			task.C = (int)atoi(temp + 4);
			task.D = (int)atoi(temp + 6);
			task.T = (int)atoi(temp + 8);
			task.remaining_C = task.C;
			arrival_que.push(task);
		}
	}
}

template <class T>
void simulate(T &task_que)
{
	int curt_time = 0;
	bool deadline_miss = false, task_table[6]{};
	Task prev_task;
	while (curt_time <= simu_time)
	{
		while (curt_time == arrival_que.top().R)
		{
			Task curt_arri = arrival_que.top();
			arrival_que.pop();
			cout << setw(3) << curt_time << "  t" << curt_arri.tid << ": arrive" << endl;
			// the arrival task do not finish the previous task
			if (task_table[curt_arri.tid] == 1)
			{
				cout << setw(3) << curt_time << "  t" << curt_arri.tid << ": deadline miss" << endl;
				deadline_miss = true;
				break;
			}
			// add new task
			task_table[curt_arri.tid] = 1;
			task_que.push(curt_arri);
			// the next arrival time
			curt_arri.R += curt_arri.T;
			arrival_que.push(curt_arri);
		}
		if (deadline_miss)
			break;
		// the previous task is preempted by current task
		if (!task_que.empty() && prev_task.tid && prev_task.tid != task_que.top().tid)
			cout << setw(3) << curt_time << "  t" << prev_task.tid << ": pause (remaining CPU burst: " << prev_task.remaining_C << ")" << endl;
		while (!task_que.empty())
		{
			Task curt_task;
			curt_task = task_que.top();
			task_que.pop();
			if (prev_task.tid != curt_task.tid)
				cout << setw(3) << curt_time << "  t" << curt_task.tid << ": start" << endl;
			curt_task.working = true;
			prev_task = curt_task;
			// task finish the remaining
			curt_time += curt_task.remaining_C;
			// task arrive stop to handle it first
			if (curt_time > arrival_que.top().R)
			{
				prev_task.remaining_C = curt_task.remaining_C = curt_time - arrival_que.top().R;
				task_que.push(curt_task);
				curt_time = arrival_que.top().R;
				break;
			}
			prev_task = Task();
			task_table[curt_task.tid] = 0;
			cout << setw(3) << curt_time << "  t" << curt_task.tid << ": end" << endl;
		}
		// finish all task before the new task arrive
		curt_time = arrival_que.top().R;
	}
}
