/**
 * @file discreteSim.cpp
 * @author Adam Hos <xhosad00>
 * @brief
 *
 *
 */

#include "discreteSim.hpp"

const int IgnoreID = -1;

namespace discSim
{

    Event::Event(int proc, int next, int fac, double start, int prio, double created) : processID(proc), processNextState(next), facilityID(fac), startTime(start), priority(prio), timeCreated(created){};
    Event::Event() // create an empty event
    {
        processID = IgnoreID;
        processNextState = -1;
        facilityID = IgnoreID;
        startTime = -1;
        priority = -1;
        timeCreated = -1;
    }

    bool Event::isProcessEvent()
    {
        return processID != IgnoreID;
    }

    Process::Process(int st, void (*b)(int), std::shared_ptr<Simulation>) : state(st), behav(b)
    {
        static int IDcntr = 0;
        id = IDcntr;
        IDcntr++;
        behav = b;
    }

    void Process::doBehavior()
    {
        behav(state);
    }

    Facility::Facility(std::string n, int cap) : name(n), capacity(cap)
    {
        static int IDcntr = 0;
        id = IDcntr;
        IDcntr++;

        stats.processCnt = 0;
        stats.waitTimeTotal = 0;
        stats.usedTimeTotal = 0;

        // if (name == "") // no name, set it to ID
        // {
        //     name = std::to_string(id);
        // }
    };

    std::ostream &operator<<(std::ostream &os, const Facility &f) // override print funciton
    {
        os << "Facility: " << f.name << "\n  processCnt   :" << f.stats.processCnt << "\n  waitTimeTotal:" << f.stats.waitTimeTotal << "\n  usedTimeTotal:" << f.stats.usedTimeTotal << "\n";
        return os;
    }

    Simulation::Simulation()
    {
        time = 0;
        sharedThis = std::shared_ptr<Simulation>(this);
    }
    void Simulation::t()
    {
        std::cout << "T\n";
    }

    void Simulation::addEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated)
    {
        // Event e = Event(processID, facilityID, startTime, priority, timeCreated);
        // calendar.push(e);
    }

    void Simulation::addEvent(Event e)
    {
        calendar.push(e);
    }

    void Simulation::addProcessEvent(int processID, int processNextState, int startTime, int priority, double timeCreated)
    {
        // Event e = Event(processID, IgnoreID, startTime, priority, this->time);
        // calendar.push(e);
    }

    Event Simulation::nextEvent()
    {
        Event e = calendar.top();
        calendar.pop();
        this->time = e.startTime;
        return e;
    }

    bool Simulation::finished()
    {
        return calendar.empty();
    }

    void Simulation::createProcess(void (*behav)(int), int state, int prio)
    {
        Process p = Process(state, behav, this->sharedThis);
        procMap.insert({p.id, p});
        calendar.emplace(p.id, state, IgnoreID, this->time, prio, this->time); 
    }


    void Simulation::executeEvent(Event e)
    {
        if (e.isProcessEvent())
        {
            std::unordered_map<int, Process>::const_iterator i = procMap.find(e.processID);
            if (i == procMap.end())
            {
                std::cout << "Could not find process: " << e.processID << "  in execute\n";
            }
            else
            {
                std::cout << "Executing process: " << i->second.id << "  " << i->second.state << "->" << e.processNextState << "\n";
            }
        }
    }

    std::shared_ptr<Simulation> Simulation::create()
    {
        std::cout << "Creating shared Sim\n";
        return std::shared_ptr<Simulation>(new Simulation());
    }

    // Simulation::~Simulation()
    // {
    //     std::cout << "Deleting sim\n";
    // }; //TODO

} // namespace