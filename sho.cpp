/**
 * @file sho.cpp
 * @author Adam Hos <xhosad00>
 * @brief 
 * 
 * 
 */

#include "discreteSim.cpp"
#include <iostream>
#include <random>
#include <string>



using namespace discSim;



unsigned int SEED = 5; //random number from Hardware
// TODo parse from args? gen seed at random

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


void testBehavior(int state) {
    std::cout << "Behavior function, state: " << state << std::endl;
}



int main(int argc, char* argv[])
{
    Simulation* sim = new Simulation();
    Event e1 = Event (0, 1, -1, 5, 20, 2);
    Event e2 = Event (1, 1, -1, 5, 20, 1);

    sim->createProcess(testBehavior, 0);

    while (!sim->finished())
    {
        Event e = sim->nextEvent();
        sim->executeEvent(e);
    }

    Facility f1 = Facility("fac1", 3);
    // std::cout << f1;

    std::cout << "Ending main\n";
    return 0;
}