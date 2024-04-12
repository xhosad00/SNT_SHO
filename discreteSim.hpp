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
#include <random>
#include <string>
#include <unordered_map>
#include <type_traits>



// namespace discSim 
// {
    
    const int CREATE_PROCESS_PRIO = 50;
    const int ACTIVATY_PROCESS_PRIO = 50;

    
    double uniformDis(double a, double b);
    double expDis(double lambd);
    double normalDis(double mean, double stddev);

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

        bool canProcessEvent();
        bool isProcessEvent();
        bool isFacilityEvent();
    };

    class Process
    {
    public:
        int id;
        int state = 0;
        void* data;
        void (*behav)(Process*, int);
        Simulation* sim;

        Process(int st, void (*b)(Process*, int), Simulation* sm = nullptr, void* data = nullptr);

        void setBehavior(void (*function)(Process*, int));
        void doBehavior();
    };


    class Facility
    {
    public:
        enum class GenType {Uniform, Exp, Normal};
        // Facility(std::string n = "", int cap = 1, GenType g = GenType::Uniform, double a = 0, double b = 0);
        
        Facility(int id, std::string n, int cap, GenType g , double a , double );
        friend std::ostream& operator<<(std::ostream& os, const Facility& f);
        int getId();

        // void processEnter(const Process& proc);
        
    private:
        struct FacilityStats
        {
            int processCnt;
            int waitTimeTotal;
            int usedTimeTotal;
        };

        int id;
        std::string name;
        int capacity;
        struct FacilityStats stats;
        //gen type and values for generating
        GenType gen;
        double a;
        double b;
        
        std::queue<int> q; //queue of Process IDs that wish to enter
    };



    class Simulation 
    {
    private:
        double time;
        double endTime;
    public:
        std::priority_queue<Event> calendar;
        // std::shared_ptr<Simulation> sharedThis;
        std::unordered_map<int, Process> procMap;
        std::unordered_map<int, Facility> facMap;

        Simulation();

        
        double getTime ();
        void setEndTime(double time);

        // void addEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated);
        // void addEvent(Event e);
        // void addProcessEvent(int processID, int processNextState, int startTime, int priority, double timeCreated);
        Event nextEvent();
        bool finished();

        void createProcess(void (*behav)(Process*, int), int state = 0, int prio = CREATE_PROCESS_PRIO, void* data = nullptr);
        // void createProcessDelayed(void (*behav)(Process*, int), int state = 0, int prio = CREATE_PROCESS_PRIO, double delay = 0);
        void executeEvent(Event e);

        void activate(int processID, int state,  int prio = ACTIVATY_PROCESS_PRIO);
        void waitFor(int processID, int state, double delay,  int prio = ACTIVATY_PROCESS_PRIO);
        // void sleepUntil(int processID, int state, double start,  int prio = ACTIVATY_PROCESS_PRIO);
        

        void createFacility(Facility f);
        // static std::shared_ptr<Simulation> create();
        
        // ~Simulation(); //TODO
    };



// } // namespace

#endif

    
