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

    /**
     * @brief Construct a new Event:: Event object
     * 
     * @param proc process ID or -1 if custom event
     * @param next next process state or -1 if custom event
     * @param fac facility ID or -1 if (not facility process or custom event)
     * @param start start time
     * @param prio event priority
     * @param created 
     */
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

    /**
     * @brief if can process event 
     * 
     * @return true if is process or facility event
     * @return false if custom event
     */
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

    /**
     * @brief Construct a new Process:: Process object
     * 
     * @param st start state
     * @param b behavior function
     * @param sm pointer to Simulation
     * @param data aditional process data
     */
    Process::Process(int st, void (*b)(Process *, void*), Simulation *sm, void *data)
    {
            static int IDcntr = 0;
            id = IDcntr;
            IDcntr++;
            behav = b;
            sim = sm;
            data = data;
    }

    /**
     * @brief set new behavior function
     * 
     * @param function 
     */
    void Process::setBehavior(void (*function)(Process *, void*))
    {
        behav = function;
    }

    /**
     * @brief call process behavior function
     * 
     */
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

    /**
     * @brief Process siezes facility. Either starts working or waits in queue
     * 
     * @param facID ID of facility
     * @param nextState processes state after exiting facility
     * @param prio priority of sieze event and activation event after exiting facility
     */
    void Process::seize(int facID, int nextState, int prio)
    {
        this->sim->seizeFacility(this->id, nextState, facID, prio);
    }

    /**
     * @brief Construct a new Facility:: Facility object
     * 
     * @param id facility ID
     * @param n name
     * @param cap capacity (how many processer can work at the same time)
     * @param g type of generating work time
     * @param a first value for Generating time
     * @param b second value for Generating Ttime
     */
    Facility::Facility(int id, std::string n, int cap, GenType g, double a, double b) : id(id), name(n), capacity(cap), gen(g), a(a), b(b)
    {
        stats.processCnt = 0;
        stats.waitTimeTotal = 0;
        stats.workTimeTotal = 0;
        if (gen == GenType::Uniform && a > b)
            throw std::invalid_argument("Uniform distribution attribute 'a' cannot be less than 'b'");
    }

    int Facility::getId()
    {
        return this->id;
    }

    // override print funciton
    std::ostream &operator<<(std::ostream &os, const Facility &f) 
    {
        os << "Facility: " << f.name << "\n  processCnt   :" << f.stats.processCnt << "\n  waitTimeTotal:" << f.stats.waitTimeTotal << "\n  workTimeTotal:" << f.stats.workTimeTotal << "\n";
        return os;
    }



    /**
     * @brief generatime time value based on facilitys GenType and gen values (a,b)
     * 
     * @return double 
     */
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
    
    /**
     * @brief process starts working, generate time and add event when it finishes
     * 
     * @param proc 
     * @param nextState 
     */
    void Facility::activateProcess(Process* proc, int nextState)
    {
        double delay = generateTime();
        double time = proc->sim->getTime();
        if (proc)
        {
            proc->sim->addFacilityEvent(proc->id, nextState, this->id, time + delay, EXIT_FACILITY_PRIO, time); 
        }
        
        //update stats
        this->stats.workTimeTotal += time;
    };

    void Facility::ProcessExit(Process *proc, Event e)
    {
        //TODO stats
        proc->state = e.processNextState;
        proc->doBehavior();
        if (this->q.empty())
            this->capacity++;
        else 
        {
            ProcInQueue inQueue = this->q.front();
            this->q.pop();
            if (Verbose)
                std::cout << "  Proc:" << inQueue.p->id << " start work in Facility: " << this->id<< "\n";
            this->activateProcess(inQueue.p, inQueue.processNextState);
            //update stats
            this->stats.waitTimeTotal += e.startTime - inQueue.enteredQueueTime;
        }
    }

    void Facility::printStats()
    {
        printf("%2d: %s\n", id, name.c_str());
        printf("  process count: %d\n", stats.processCnt);
        printf("  work time total: %.3lf\n", stats.workTimeTotal);
        printf("  wait time total: %.3lf\n", stats.waitTimeTotal);
    }

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

    /**
     * @brief create a process and sets it's activation time to current Simulation time
     * 
     * @param behav process behavior function
     * @param state process initial state
     * @param prio activation event priority
     * @param data process aditional data
     */
    void Simulation::createProcess(void (*behav)(Process*, void*), int state, int prio, void* data) 
    {
        Process p = Process(state, behav, this, nullptr); 
        procMap.emplace(p.id, p);
        calendar.emplace(p.id, state, IgnoreID, this->time, prio, this->time); 
    }

    /**
     * @brief create a process and sets it's activation time to (current Simulation time + delay)
     * 
     * @param delay
     * @param behav process behavior function
     * @param state process initial state
     * @param prio activation event priority
     * @param data process aditional data
     */
    void Simulation::createProcessDelayed(double delay, void (*behav)(Process *, void *), int state, int prio, void *data)
    {
        Process p = Process(state, behav, this, nullptr); 
        procMap.emplace(p.id, p);
        calendar.emplace(p.id, state, IgnoreID, this->time + delay, prio, this->time); 
    }

    /**
     * @brief create a process and sets it's activation time to time from args
     * 
     * @param time time when process will be activated
     * @param behav process behavior function
     * @param state process initial state
     * @param prio activation event priority
     * @param data process aditional data
     * @return true if sucessfuly created
     * @return false if activation time < Simulation.time
     */
    bool Simulation::createProcessAtTime(double time, void (*behav)(Process *, void *), int state, int prio, void *data)
    {
        if (this->time > time)
            return false;            
        Process p = Process(state, behav, this, nullptr); 
        procMap.emplace(p.id, p);
        calendar.emplace(p.id, state, IgnoreID, time, prio, this->time); 
        return true;
    }


    Event* Simulation::executeEvent(Event e)
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
                // std::cout << " proc: " << p.id << "  " << p.state << "->" << e.processNextState << "\n";
                if (Verbose)
                    std::cout << "  " << p.state << "->" << e.processNextState << "\n";
                p.state = e.processNextState;
                p.doBehavior();
            }
            return nullptr;
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
                Facility* f = &fi->second;
                Process* p = &pi->second;
                if (Verbose)
                    std::cout << "  exiting facility:" << f->id << "\n";
                f->ProcessExit(p, e); // TODO test
            }
            return nullptr;
        }
        return new Event(e); // TODo check
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
            
            //update stats        
            f->stats.processCnt++;
            if (f->capacity > 0) // processed starts working
            {
                f->capacity--;
                if (Verbose)
                    std::cout << " start work in Facility: " << f->getId() << "\n";
                f->activateProcess(p, state);                
            }
            else    //enter queue
            {
                if (Verbose)
                    std::cout << " queue enter in Facility: " << f->getId() << "\n";
                Facility::ProcInQueue pq = {p, state, this->time};
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

    /**
     * @brief Construct a new Facility and place it into simulation facility map
     * 
     * @param id facility ID
     * @param n name
     * @param cap capacity (how many processer can work at the same time)
     * @param g type of generating work time
     * @param a first value for Generating time
     * @param b second value for Generating Ttime
     */
    void Simulation::createFacility(int id, std::string n, int cap, Facility::GenType g, double a, double b)
    {
        this->facMap.emplace(id, Facility(id, n, cap, g, a, b));
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

    void Simulation::printFacilitysStats()
    {
        printf("\n----PRINT FACILITY STATS----\n");
        for (auto& i : facMap) {
            // Call printStats() on the Facility object associated with each key
            i.second.printStats();
        }
    }

    // Simulation::~Simulation()
    // {
    //     std::cout << "Deleting sim\n";
    // }; //TODO

// } // namespace


