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
        return processID != IgnoreID && facilityID == IgnoreID;
    }

    bool Event::isFacilityEvent()
    {
        return facilityID != IgnoreID;
    }

    
    Process::Process(int st, void (*b)(Process *, void*), Simulation *sm, void *data)
    {
            static int IDcntr = 0;
            id = IDcntr;
            IDcntr++;
            behav = b;
            sim = sm;
            data = data;
    }

    void Process::setBehavior(void (*function)(Process *, void*))
    {
        behav = function;
    }

    void Process::doBehavior()
    {
        if (behav) 
        {
            behav(this, &this->state);
        }
        else
        {
            std::cerr << "Process:" << this->id << "  does not have set Behavior\n";
        }        
    }

    void Process::seize(int facID, int nextState, int prio)
    {
        this->sim->seizeFacility(this->id, nextState, facID, prio);
    }

    Facility::Facility(int id, std::string n, int cap, GenType g, double a, double b) : id(id), name(n), capacity(cap), gen(g), a(a), b(b)
    {
        stats.processCnt = 0;
        stats.waitTimeTotal = 0;
        stats.usedTimeTotal = 0;
        if (gen == GenType::Uniform && a > b)
            throw std::invalid_argument("Uniform distribution attribute 'a' cannot be less than 'b'");
    }

    int Facility::getId()
    {
        return this->id;
    }

    std::ostream &operator<<(std::ostream &os, const Facility &f) // override print funciton
    {
        os << "Facility: " << f.name << "\n  processCnt   :" << f.stats.processCnt << "\n  waitTimeTotal:" << f.stats.waitTimeTotal << "\n  usedTimeTotal:" << f.stats.usedTimeTotal << "\n";
        return os;
    }


    void Facility::ProcessExit(Process *proc, Event e)
    {
        //TODO stats
        // std::cout << " Executing process: " << proc->id << "  " << proc->state << "->" << e.processNextState << "\n";
        proc->state = e.processNextState;
        proc->doBehavior();
    }

    double Facility::generateTime()
    {
        switch(this->gen)
        {
            case Facility::GenType::Exp:
                return expDis(this->a);

            case Facility::GenType::Normal:
                return normalDis(this->a, this->b);

            case Facility::GenType::Uniform:
            default:
                return uniformDis(this->a, this->b);
                break;
        }
    }
    
    void Facility::activateProcess(Process* proc, int nextState)
    {
        double delay = generateTime();
        double time = proc->sim->getTime();
        if (proc)
        {
            proc->sim->addFacilityEvent(proc->id, nextState, this->id, time + delay, EXIT_FACILITY_PRIO, time); 
        }

    };

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

    void Simulation::addProcessEvent(int processID, int processNextState, double startTime, int priority, double timeCreated)
    {
        Event e = Event(processID, processNextState, IgnoreID, startTime, priority, this->time);
        calendar.push(e);
    }

    void Simulation::addFacilityEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated)
    {
        Event e = Event(processID, processNextState, facilityID, startTime, priority, timeCreated);
        calendar.push(e);
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
        return calendar.empty() || (this->endTime > 0 && this->time > this->endTime);
    }

    void Simulation::createProcess(void (*behav)(Process*, void*), int state, int prio, void* data) 
    {
        Process p = Process(state, behav, this, nullptr);
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
        else if (e.isFacilityEvent())
        {
            std::unordered_map<int, Facility>::iterator fi = facMap.find(e.facilityID);
            std::unordered_map<int, Process>::iterator pi = procMap.find(e.processID);
            if (fi == facMap.end())
            {
                std::cerr << " Could not find Facility: " << e.processID << "  in execute\n";
            }
            else if (pi == procMap.end())
            {
                std::cerr << " Could not find process: " << e.processID << "  in execute\n";
            }
            else
            {
                auto f = fi->second;
                Process* p = &pi->second;
                f.ProcessExit(p, e); // TODO test
            }
        }
        //TODO generic event?
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

    void Simulation::seizeFacility(int processID, int state, int facilityID, int prio)
    {
        std::unordered_map<int, Facility>::iterator fi = facMap.find(facilityID); // TODO switch to find
        std::unordered_map<int, Process>::iterator pi = procMap.find(processID);
        if (fi == facMap.end())
        {
            std::cerr << " Could not find Facility: " << facilityID << "  in seizeFacility\n";
        }
        else if (pi == procMap.end())
        {
            std::cerr << " Could not find process: " << processID << "  in seizeFacility\n";
        }
        else
        {
            Facility* f = &fi->second;
            Process* p = &pi->second;
            
            f->stats.processCnt++;
            if (f->capacity > 0) // processed starts working
            {
                f->capacity--;
                f->activateProcess(p, state);
                
            }
            else    //enter queue
            {            
                Facility::ProcInQueue pq = {p, state};
                f->q.push(pq);
            }
            // 
        }
    }

    void Simulation::createFacility(Facility f)
    {
        // faci.emplace(p.id, state, IgnoreID, this->time, prio, this->time); 
        this->facMap.emplace(f.getId(), f);
    }

    Facility *Simulation::findFacility(int id)
    {
        std::unordered_map<int, Facility>::iterator fi = facMap.find(id); 
        if (fi == facMap.end())
        {
            std::cerr << " Could not find Facility: " << id << "\n";
            return nullptr;
        }
        return &fi->second;
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


