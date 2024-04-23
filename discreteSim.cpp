/**
 * @file discreteSim.cpp
 * @author Adam Hos <xhosad00>
 * @brief
 *
 *
 */

#include "discreteSim.hpp"

unsigned int SEED = 0; 
// TODo parse from args? gen seed at random

// namespace discSim
// {

/**********DISTRIBUTIONS**********/


    /**
     * @brief Parse seed from arguments if it was set
     * 
     * @param argc 
     * @param argv 
     * @param seed Teference to variable where seed will be stored
     * @return 0 if valid arguments / none oarguments ; return 1 if incorrect format
     */
    int parseArguments(int argc, char *argv[], unsigned int &seed) 
    {
        if (argc > 1) {
            std::string arg = argv[1];         
            if (arg == "--seed") {
                if (argc > 2) {
                    seed = std::atoi(argv[2]);
                    return 0; 
                } else {
                    std::cerr << "No seed value provided." << std::endl;
                    return 1; 
                }
            } else {
                std::cerr << "Invalid argument. Use '--seed' <int>" << std::endl;
                return 1; 
            }
        }
        return 0;
    }

    /**
     * @brief Generates a random number from a uniform distribution between a and b
     * 
     * @param a The lower bound of the uniform distribution
     * @param b The upper bound of the uniform distribution
     * @return A random number from the uniform distribution
     */
double uniformDis(double a, double b)
{
    static std::default_random_engine gen(SEED);
    std::uniform_real_distribution<double> dis(a, b);

    return dis(gen);
    }
    /**
     * @brief Generates a random number from an exponential distribution with parameter lambda
     * 
     * @param lambd The rate parameter of the exponential distribution
     * @return A random number from the exponential distribution
     */
    double expDis(double lambd)
    {
        static std::default_random_engine gen(SEED);
        std::exponential_distribution<double> dis(lambd);

        return dis(gen);
    }
    /**
     * @brief Generates a random number from a normal distribution with specified mean and standard deviation
     * 
     * @param mean The mean of the normal distribution
     * @param stddev The standard deviation of the normal distribution
     * @return A random number from the normal distribution
     */
    double normalDis(double mean, double stddev)
    {
        static std::default_random_engine gen(SEED);
        std::normal_distribution<double> dis(mean, stddev);
        return dis(gen);
    }
/**********DISTRIBUTIONS**********/


/**********EVENT**********/
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
    /**
     * @brief Default constructor for Event class
     */
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
     * @brief check if simulation can process this event 
     * 
     * @return true if is process or facility event
     * @return false if custom event
     */
    bool Event::canProcessEvent()
    {
        return this->isProcessEvent() || this->isFacilityEvent();
    }

    /**
     * @brief Checks if the event is a process event
     * 
     * @return true if the event is a process event, false otherwise
     */
    bool Event::isProcessEvent()
    {
        return processID != IgnoreID && facilityID == IgnoreID;
    }

    /**
     * @brief Checks if the event is a facility event
     * 
     * @return true if the event is a facility event, false otherwise
     */
    bool Event::isFacilityEvent()
    {
        return facilityID != IgnoreID;
    }
/**********EVENT**********/


/**********PROCESS**********/
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
/**********PROCESS**********/


/**********FACILITY**********/
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

    /**
     * @brief Get the ID of the facility
     * 
     * @return The ID of the facility
     */
    int Facility::getId()
    {
        return this->id;
    }

    /**
     * @brief Overloaded stream insertion operator for printing Facility objects
     * 
     * @param os The output stream
     * @param f The Facility object to be printed
     * @return The output stream
     */
    std::ostream &operator<<(std::ostream &os, const Facility &f) 
    {
        os << "Facility: " << f.name << "\n  processCnt   :" << f.stats.processCnt << "\n  waitTimeTotal:" << f.stats.waitTimeTotal << "\n  workTimeTotal:" << f.stats.workTimeTotal << "\n";
        return os;
    }



    /**
     * @brief generatime time value based on facilitys GenType and gen values (a,b)
     * 
     * @return The generated time value 
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

    /**
     * @brief Handle process exit from the facility
     * 
     * @param proc Pointer to the process exiting the facility
     * @param e The event triggering the process exit
     */
    void Facility::ProcessExit(Process *proc, Event e)
    {
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

    /**
     * @brief Print statistics related to the facility
     */
    void Facility::printStats()
    {
        printf("%2d: %s\n", id, name.c_str());
        printf("  process count: %d\n", stats.processCnt);
        printf("  work time total: %.3lf\n", stats.workTimeTotal);
        printf("  wait time total: %.3lf\n", stats.waitTimeTotal);
    }

/**********FACILITY**********/



/**********SIMULATION**********/
    /**
     * @brief Default constructor for Simulation class
     */
    Simulation::Simulation()
    {
        time = 0;
        endTime = -1;
        // sharedThis = std::shared_ptr<Simulation>(this);
    }

    /**
     * @brief Get the current simulation time
     * 
     * @return The current simulation time
     */
    double Simulation::getTime()
    // double Simulation::getTime()
    {
        return this->time;
    }

    /**
     * @brief Set the end time of the simulation
     * 
     * @param time The end time of the simulation
     */
    void Simulation::setEndTime(double time)
    {
        this->endTime = time;
    }

    /**
     * @brief Add an event to the simulation
     * 
     * @param processID The ID of the process associated with the event
     * @param processNextState The next state of the associated process
     * @param facilityID The ID of the facility associated with the event
     * @param startTime The start time of the event
     * @param priority The priority of the event. Higher priority is better (100 is important, 0 is less)
     * @param timeCreated The time when the event was created
     */
    void Simulation::addEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated)
    {
        calendar.emplace(processID, processNextState, facilityID, startTime, priority, timeCreated);
    }

    /**
     * @brief Add an event to the simulation
     * 
     * @param e The event to be added
     */
    void Simulation::addEvent(Event e)
    {
        calendar.push(e);
    }

    /**
     * @brief Add a process event to the simulation
     * 
     * @param processID The ID of the process associated with the event
     * @param processNextState The next state of the associated process
     * @param startTime The start time of the event
     * @param priority The priority of the event. Higher priority is better (100 is important, 0 is less)
     * @param timeCreated The time when the event was created
     */
    void Simulation::addProcessEvent(int processID, int processNextState, double startTime, int priority, double timeCreated)
    {
        Event e = Event(processID, processNextState, IgnoreID, startTime, priority, this->time);
        calendar.push(e);
    }

    /**
     * @brief Add a facility event to the simulation
     * 
     * @param processID The ID of the process associated with the event
     * @param processNextState The next state of the associated process
     * @param facilityID The ID of the facility associated with the event
     * @param startTime The start time of the event
     * @param priority The priority of the event. Higher priority is better (100 is important, 0 is less)
     * @param timeCreated The time when the event was created
     */
    void Simulation::addFacilityEvent(int processID, int processNextState, int facilityID, double startTime, int priority, double timeCreated)
    {
        Event e = Event(processID, processNextState, facilityID, startTime, priority, timeCreated);
        calendar.push(e);
    }

    /**
     * @brief Get the next event from the event calendar.
     * 
     * @return The next event from the event calendar.
     */
    Event Simulation::nextEvent()
    {
        Event e = calendar.top();
        calendar.pop();
        this->time = e.startTime;
        return e;
    }

    /**
     * @brief Check if the simulation has finished.
     * 
     * @return true if the simulation has finished, false otherwise.
     */
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

    /**
     * @brief Execute an event in the simulation
     * 
     * @param e The event to be executed
     * @return Pointer to the executed event
     */
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

    /**
     * @brief Activate a process in the simulation at current sim time
     * 
     * @param processID The ID of the process to be activated
     * @param state The state to which the process should transition after activation
     * @param prio Priority of the process activation
     */
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

    /**
     * @brief Make a process wait for a specified time in the simulation
     * 
     * @param processID The ID of the process to wait
     * @param state The state to which the process should transition after waiting
     * @param delay The time to wait
     * @param prio Priority of the process waiting
     */
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

    /**
     * @brief Seize a facility by a process in the simulation
     * 
     * @param processID The ID of the process seizing the facility
     * @param state The state to which the process should transition after seizing the facility
     * @param facilityID The ID of the facility to be seized
     * @param prio Priority of the process seizing the facility
     */
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

    /**
     * @brief Create a facility and add it to the simulation
     * 
     * @param f The facility to be created
     */
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

    /**
     * @brief Find a facility by its ID in the simulation
     * 
     * @param id The ID of the facility to find
     * @return Pointer to the facility if found, nullptr otherwise
     */
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

    /**
     * @brief Print statistics of all facilities in the simulation
     */
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


