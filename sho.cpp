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

void testBehavior(Process* p, int state) 
{
    int x = 0;
    switch ((state))
    {
    case 0:
        std::cout << "Behavior, state: " << state << std::endl;
        p->sim->activate(p->id, 1);
        // std::cout << p->sim->time;
        break;
    case 1:
        std::cout << "Behavior2, state: " << state << std::endl;
        p->sim->waitFor(p->id, 2, 3.5);
        break;
    case 2:
        std::cout << "Behavior3, state: " << state << std::endl;
        break;
    
    default:
        break;
    }    
}

void testBehaviorPrint(Process* p, int state) 
{
    std::cout << "Behavior PRINT, state: " << state << std::endl;
}



int main(int argc, char* argv[])
{
    Simulation* sim = new Simulation();

    sim->createProcess(testBehavior, 0);
    Facility f0(0, "Shopping", 10, Facility::GenType::Normal, 10, 5);
    sim->createFacility(f0);

    while (!sim->finished())
    {
        Event e = sim->nextEvent();
        if (e.canProcessEvent())
        {
            if (VERBOSE)
                printf("%2.1lf: Executing Process:%d\n", sim->getTime(), e.processID);
            sim->executeEvent(e);
        }
    }



    std::cout << "Ending main\n";
    return 0;
}