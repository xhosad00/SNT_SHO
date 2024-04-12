/**
 * @file discreteSim.cpp
 * @author Adam Hos <xhosad00>
 * @brief
 *
 *
 */

#include "discreteSim.hpp"

const int IgnoreID = -1;
unsigned int SEED = 5; //random number from Hardware
// TODo parse from args? gen seed at random

// namespace discSim
// {

    double uniformDis(double a, double b)
    {
        static std::default_random_engine gen(SEED);
        std::uniform_real_distribution<double> dis(a, b);

        return dis(gen);
    }

    double expDis(double lambd)
    {
        static std::default_random_engine gen(SEED);
        std::exponential_distribution<double> dis(lambd);

        return dis(gen);
    }

    double normalDis(double mean, double stddev)
    {
        static std::default_random_engine gen(SEED);
        std::normal_distribution<double> dis(mean, stddev);
        return dis(gen);
    }

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

    bool Event::canProcessEvent()
    {
        return this->isProcessEvent() || this->isFacilityEvent();
    }

    bool Event::isProcessEvent()
    {
        return processID != IgnoreID;
    }

    bool Event::isFacilityEvent()
    {
        return facilityID != IgnoreID;
    }

    
    Process::Process(int st, void (*b)(Process *, int), Simulation *sm, void *data)
    {
            static int IDcntr = 0;
            id = IDcntr;
            IDcntr++;
            behav = b;
            sim = sm;
            data = data;
    }

    void Process::setBehavior(void (*function)(Process *, int))
    {
        behav = function;
    }

    void Process::doBehavior()
    {
        if (behav) 
        {
            behav(this, this->state);
        }
        else
        {
            std::cerr << "Process:" << this->id << "  does not have set Behavior\n";
        }        
    }

    Facility::Facility(int id, std::string n, int cap, GenType g, double a, double b) : id(id), name(n), capacity(cap), gen(g), a(a), b(b)
    {
        stats.processCnt = 0;
        stats.waitTimeTotal = 0;
        stats.usedTimeTotal = 0;

        // if (name == "") // no name, set it to ID
        // {
        //     name = std::to_string(id);
        // }
    }

    int Facility::getId()
    {
        return this->id;
    }

    // std::ostream &operator<<(std::ostream &os, const Facility &f) // override print funciton
    // {
    //     os << "Facility: " << f.name << "\n  processCnt   :" << f.stats.processCnt << "\n  waitTimeTotal:" << f.stats.waitTimeTotal << "\n  usedTimeTotal:" << f.stats.usedTimeTotal << "\n";
    //     return os;
    // }

    
    // void Facility::processEnter(const Process &proc)
    // {
        
    // };



    
    Simulation::Simulation()
    {
        time = 0;
        endTime = -1;
        // sharedThis = std::shared_ptr<Simulation>(this);
    }


    double Simulation::getTime()
    // double Simulation::getTime()
    {
        return this->time;
    }

    void Simulation::setEndTime(double time)
    {
        this->endTime = time;
    }

    // void Simulation::addEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated)
    // {
    //     // Event e = Event(processID, processNextState, facilityID, startTime, priority, timeCreated);
    //     calendar.emplace(processID, processNextState, facilityID, startTime, priority, timeCreated);
    // }

    // void Simulation::addEvent(Event e)
    // {
    //     calendar.push(e);
    // }

    // void Simulation::addProcessEvent(int processID, int processNextState, int startTime, int priority, double timeCreated)
    // {
    //     // Event e = Event(processID, IgnoreID, startTime, priority, this->time);
    //     // calendar.push(e);
    // }

    Event Simulation::nextEvent()
    {
        Event e = calendar.top();
        calendar.pop();
        this->time = e.startTime;
        return e;
    }

    bool Simulation::finished()
    {
        return calendar.empty() || this->time > this->endTime;
    }

    void Simulation::createProcess(void (*behav)(Process*, int), int state, int prio, void* data) 
    {
        Process p = Process(state, behav, this, data);
        procMap.emplace(p.id, p);
        // procMap[p.id] = p;
        calendar.emplace(p.id, state, IgnoreID, this->time, prio, this->time); 
    }


    void Simulation::executeEvent(Event e)
    {
        if (e.isProcessEvent())
        {
            std::unordered_map<int, Process>::iterator i = procMap.find(e.processID);
            if (i == procMap.end())
            {
                std::cerr << " Could not find process: " << e.processID << "  in execute\n";
            }
            else
            {
                auto p = i->second;
                std::cout << " Executing process: " << p.id << "  " << p.state << "->" << e.processNextState << "\n";
                p.state = e.processNextState;
                p.doBehavior();
            }
        }
        else // facility event
        {

        }
    }

    void Simulation::activate(int processID, int state, int prio)
    {
        std::unordered_map<int, Process>::const_iterator i = procMap.find(processID);
        auto p = i->second;
        if (i == procMap.end())
        {
            std::cerr << "Could not find process: " << processID << "  in activate\n";
        }
        calendar.emplace(p.id, state, IgnoreID, this->time, prio, this->time); 
    }

    void Simulation::waitFor(int processID, int state, double delay, int prio)
    {        
        std::unordered_map<int, Process>::const_iterator i = procMap.find(processID);
        auto p = i->second;
        if (i == procMap.end())
        {
            std::cerr << "Could not find process: " << processID << "  in waitFor\n";
        }
        calendar.emplace(p.id, state, IgnoreID, this->time + delay, prio, this->time);         
    }

    void Simulation::createFacility(Facility f)
    {
        // faci.emplace(p.id, state, IgnoreID, this->time, prio, this->time); 
        this->facMap.emplace(f.getId(), f);
    }

    // std::shared_ptr<Simulation> Simulation::create()
    // {
    //     std::cout << "Creating shared Sim\n";
    //     return std::shared_ptr<Simulation>(new Simulation());
    // }

    // Simulation::~Simulation()
    // {
    //     std::cout << "Deleting sim\n";
    // }; //TODO

// } // namespace


