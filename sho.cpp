/**
 * @file sho.cpp
 * @author Adam Hos <xhosad00>
 * @brief 
 * 
 * 
 */

#include "discreteSim.cpp"

// using namespace discSim;

const bool VERBOSE = true;






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
    int x = 0;
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
    // int facilityID = static_cast<int>*data;
    int x = 0;
    switch ((state))
    {
    case 0:
        std::cout << " state: " << state << std::endl;
        p->seize(10, 1, 20);
        break;
    case 1:
        std::cout << " state: " << state << std::endl;    
    default:
        break;
    }    
}

// void testBehaviorPrint(Process* p, void* data) 
// {
//     int state = (int)*data;
//     std::cout << "Behavior PRINT, state: " << state << std::endl;
// }

// void testFacility(Process* p, void* data) 
// {
//     int state = (int)*data;
//     std::cout << "Behavior PRINT, state: " << state << std::endl;
// }


int main(int argc, char* argv[])
{
    Simulation* sim = new Simulation();
    int initState = 0;

    sim->createProcess(testBehaviorFac, initState);
    Facility f0(10, "Shopping", 3, Facility::GenType::Uniform, 1, 5);
    sim->createFacility(f0);

    while (!sim->finished())
    {
        Event e = sim->nextEvent();
        if (e.canProcessEvent())
        {
            if (VERBOSE)
                int id = e.processID;
                printf("%2.1lf: Executing Process:%d\n", e.startTime, e.processID);
                int x = 2 + 3;
            sim->executeEvent(e);
        }
    }



    std::cout << "Ending main\n";
    return 0;
}