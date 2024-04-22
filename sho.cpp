/**
 * @file sho.cpp
 * @author Adam Hos <xhosad00>
 * @brief 
 * 
 * 
 */

#include "discreteSim.hpp"

// using namespace discSim;

const bool Verbose = true;



struct structExample1
{
    int x, y;
};
struct structExample2
{
    std::string name;
    double val;
};

void testBehavior(Process* p, void* data) 
{
    int state = p->state;
    // int facilityID = static_cast<int>*data;
    switch ((state))
    {
    case 0:
        std::cout << "Behavior, state: " << state << std::endl;
        p->sim->activate(p->id, 2);
        // std::cout << p->sim->time;
        break;
    case 1:
        std::cout << "Behavior2, state: " << state << std::endl;
        p->sim->waitFor(p->id, 2, 3.5);
        break;
    case 2:
        std::cout << "Behavior3, state: " << state << std::endl;
        p->seize(10, 3, 20);
        break;
    case 3:
        std::cout << "Behavior4, state: " << state << std::endl;    
    default:
        break;
    }    
}

void testBehaviorFac(Process* p, void* data) 
{
    int state = p->state;
    const int facilityID = 10;
    switch ((state))
    {
    case 0:
    {   
        // std::cout << " state: " << state << std::endl;
        const int nextState = 1;
        const int seizePrio = 20;
        p->seize(facilityID, nextState, seizePrio);
        break;
    }
    case 1:
        // std::cout << " state: " << state << std::endl;    
    default:
        break;
    }    
}


int main(int argc, char* argv[])
{
    Simulation* sim = new Simulation();
    int initState = 0;

    sim->createProcess(testBehaviorFac, initState);
    sim->createProcessAtTime(5, testBehaviorFac, initState);
    // Facility f0 = Facility;
    sim->createFacility(10, "Shopping", 1, Facility::GenType::Uniform, 8, 10);
    sim->createProcessAtTime(5, testBehaviorFac, initState, 100);

    while (!sim->finished())
    {
        Event e = sim->nextEvent();
        if (e.canProcessEvent())
        {
            if (Verbose)
                printf("%2.1lf: Proc:%d\n", e.startTime, e.processID);
            sim->executeEvent(e);
        }
    }

    sim->printFacilitysStats();

    std::cout << "Ending main\n";
    return 0;
}