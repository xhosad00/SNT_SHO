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
#include <stdexcept>


extern const bool Verbose;  ///< External boolean variable controlling verbose output
const int IgnoreID = -1;    ///< Constant indicating an ID to be ignored, used for procID and facID
extern unsigned int SEED;      ///< Seed for random number
// namespace discSim 
// {
    
    const int CREATE_PROCESS_PRIO = 60;     ///< Priority for creating a process
    const int ACTIVATE_PROCESS_PRIO = 50;   ///< Priority for activating a process
    const int SEIZE_FACILITY_PRIO = 20;     ///< Priority for seizing a facility
    const int EXIT_FACILITY_PRIO = 30;      ///< Priority for exiting a facility

    int parseArguments(int argc, char* argv[], unsigned int& seed);
    double uniformDis(double a, double b);
    double expDis(double lambd);
    double normalDis(double mean, double stddev);

    class Simulation;


    /**
     * @brief The Event class represents an event in the discrete event simulation
     * 
     * Events are scheduled to occur at specific times and are associated with
     * processes, facilities, or other entities in the simulation
     */
    class Event
    {
    public:
        int processID;          ///< The ID of the process associated with the event
        int processNextState;   ///< The next state of the associated process
        int facilityID;         ///< The ID of the facility associated with the event
        double startTime;       ///< The start time of the event
        int priority;           ///< The priority of the event. Higher priority is better. (100 is important, 0 is less)
        double timeCreated;     ///< The time when the event was created

        Event(int proc, int nxt, int fac, double start, int prio, double created);
        Event();

        /**
         * @brief Overloaded less-than operator for comparing events
         * 
         * Events are compared based on their start time, priority, and creation time
         * 
         * @param other The event to compare with
         * @return true if this event is scheduled to occur earlier than the other event, false otherwise
         */
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

    /**
     * @brief The Process class represents a discrete process in the simulation
     * 
     * Processes are entities that perform actions over time in the simulation
     */
    class Process
    {
    public:
        int id;                     ///< The ID of the process
        int state;                  ///< The current state of the process
        void* data;                 ///< Pointer to additional data associated with the process
        void (*behav)(Process*, void*);  ///< Pointer to the behavior function of the process
        Simulation* sim;            ///< Pointer to the Simulation object associated with the process
        void* buffer;               ///< Pointer to a buffer used by the process (if any)
        
        Process(int st, void (*b)(Process*, void*), Simulation* sm = nullptr, void* data = nullptr);

        void setBehavior(void (*function)(Process*, void*));
        void doBehavior();
        void seize(int facID, int nextState, int prio = SEIZE_FACILITY_PRIO);
    };

    /**
     * @brief The Facility class represents a facility in the discrete event simulation
     * 
     * Facilities are resources that can be seized by processes to perform activities
     */
    class Facility
    {
    public:
        /**
         * @brief This enum represents the types of distributions used for generating random numbers
         * 
         */
        enum class GenType {
            Uniform,    ///< Uniform distribution
            Exp,        ///< Exponential distribution
            Normal      ///< Normal distribution
        };
        
        Facility(int id, std::string n, int cap, GenType g , double a , double );
        friend std::ostream& operator<<(std::ostream& os, const Facility& f);
        int getId();

        void activateProcess(Process* proc, int nextState);
        void ProcessExit(Process* proc, Event e);
        void printStats();
        
    
        struct FacilityStats    ///< Structure to hold facility statistics
        {
            int processCnt;         ///< The count of processes using the facility
            double waitTimeTotal;   ///< The total wait time of processes at the facility
            double workTimeTotal;   ///< The total work time of the facility
        };
        struct ProcInQueue  ///< Structure to hold information about a process in the facility queue
        {
            Process* p;             ///< Pointer to the process
            int processNextState;   ///< The next state of the process
            double enteredQueueTime;    ///< The time when the process entered the queue
        };

        int id;             ///< The ID of the facility
        std::string name;   ///< The name of the facility
        int capacity;       ///< The capacity of the facility
        GenType gen;        ///< The generation type for facility usage time
        double a;           ///< The first parameter for generating facility usage time (depends on generation type)
        double b;           ///< The second parameter for generating facility usage time (depends on generation type)
        struct FacilityStats stats; ///< The Facility statistics
        std::queue<ProcInQueue> q;  ///< Queue of processes waiting to enter the facility

        double generateTime();
    };

    /**
     * @brief The Simulation class represents a discrete event simulation
     * 
     * Simulation manages the simulation time, event calendar, processes, and facilities
     */
    class Simulation 
    {
    private:
        double time;    ///< Current simulation time
        double endTime; ///< End time of the simulation, -1 if the simulation should not end on timer
    public:
        std::priority_queue<Event> calendar;        ///< Priority queue for storing simulation events
        std::unordered_map<int, Process> procMap;   ///< Map of processes in the simulation
        std::unordered_map<int, Facility> facMap;   ///< Map of facilities in the simulation


        Simulation();

        
        double getTime ();
        void setEndTime(double time);

        void addEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated);
        void addEvent(Event e);
        void addProcessEvent(int processID, int processNextState, double startTime, int priority, double timeCreated);
        void addFacilityEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated);
        Event nextEvent();
        bool finished();

        void createProcess(void (*behav)(Process*, void*), int state = 0, int prio = CREATE_PROCESS_PRIO, void* data = nullptr);
        void createProcessDelayed(double delay, void (*behav)(Process*, void*), int state = 0, int prio = CREATE_PROCESS_PRIO, void* data = nullptr);
        bool createProcessAtTime(double time, void (*behav)(Process*, void*), int state = 0, int prio = CREATE_PROCESS_PRIO, void* data = nullptr);;
        Event* executeEvent(Event e);

        void activate(int processID, int state,  int prio = ACTIVATE_PROCESS_PRIO);
        void waitFor(int processID, int state, double delay,  int prio = ACTIVATE_PROCESS_PRIO);
        void seizeFacility(int processID, int state, int facilityID,  int prio = SEIZE_FACILITY_PRIO);
        

        void createFacility(Facility f);
        void createFacility(int id, std::string n, int cap, Facility::GenType g, double a, double b);
        Facility* findFacility(int id);
        void printFacilitysStats();
        
    };



// } // namespace

#endif // DISCRETE_SIM_HPP

    
