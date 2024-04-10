/**
 * @file discreteSim.hpp
 * @author Adam Hos <xhosad00>
 * @brief 
 * 
 * 
 */

#ifndef DISCRETE_SIM_HPP
#define DISCRETE_SIM_HPP

#include <iostream>
#include <queue>
#include <memory>
#include <unordered_map>

namespace discSim 
{
    const int CREATE_PROCESS_PRIO = 50;

    class Simulation;

    class Event
    {
    public:
        int processID;
        int processNextState;
        int facilityID; //TODO how will facility be handled?
        double startTime;
        int priority;       //As of now higher priority is better (100 important, 0 less)
        double timeCreated;

        Event(int proc, int nxt, int fac, double start, int prio, double created);
        Event();

        bool operator<(const Event& other) const {
            if (startTime != other.startTime)
                return startTime > other.startTime;
            if (priority != other.priority)
                return priority < other.priority;
            return timeCreated > other.timeCreated;
        }

        bool isProcessEvent();
    };

    class Process
    {
    public:
        int id;
        int state = 2;
        std::shared_ptr<Simulation> sim;

        void (*behav)(int);

        Process(int st, void (*b)(int), std::shared_ptr<Simulation>);

        void doBehavior();
    };

    class Facility
    {
        struct FacilityStats
        {
            int processCnt;
            int waitTimeTotal;
            int usedTimeTotal;
        };
        
    private:
        int id;
        std::string name;
        int capacity;
        struct FacilityStats stats;
        
        std::queue<int> q; //queue of Process IDs that wish to enter

    public:
        Facility(std::string n = "", int cap = 1);

        friend std::ostream& operator<<(std::ostream& os, const Facility& f);
    };

    class Simulation
    {
    public:
        double time;
        std::priority_queue<Event> calendar;
        std::shared_ptr<Simulation> sharedThis;
        std::unordered_map<int, Process> procMap;

        Simulation();

        void t();

        void addEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated);
        void addEvent(Event e);
        void addProcessEvent(int processID, int processNextState, int startTime, int priority, double timeCreated);
        Event nextEvent();
        bool finished();
        void createProcess(void (*behav)(int), int state = 0, int prio = CREATE_PROCESS_PRIO);
        void createProcessDelayed(void (*behav)(int), int state = 0, int prio = CREATE_PROCESS_PRIO, double delay = 0);
        void executeEvent(Event e);

        void activate();
        void speelFor();
        void sleepUntil();
        
        static std::shared_ptr<Simulation> create();
        
        // ~Simulation();
    };



} // namespace

#endif