#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

int lcm(int n1, int n2)
{
    return n1 * n2 / gcd(n1, n2);
}

int lcm(int n1, int n2, int n3)
{
    return lcm(lcm(n1, n2), n3);
}

int main()
{
    // Parameters
    int sz = 3;                   // number of flows (or jobs)
    vector<int> net = {20, 10, 5}; // time for which it uses the network (at full bottleneck link
                                  // capacity). Corresponds to a*T in the HotNets paper
    int SIMTICKS = 200000;
    int sumnet = accumulate(net.begin(), net.end(), 0); // sum of network times

    /*

    For one iteration of MLTCP, below is the link utilisation pattern (after convergence) 

    ########### | ########## | ########## | ++++++++++++++++++ | ... (same pattern repeats)
     net[0] sec     net[1]       net[2]     x seconds (link is free)

    */

    // Iterate over x values
    for (int x = 50; x < 51; x++)
    {
        double util = 0;

        // Initialize `t` vector
        vector<int> compute_time(sz);
        compute_time[0] = sumnet + x - net[0];
        cout << "t = " << compute_time[0] << ",";
        for (int i = 1; i < sz; i++)
        {
            compute_time[i] = sumnet + x - net[i] - net[i - 1] + 1;
            cout << compute_time[i] << ",";
        }
        cout << "\n";

        vector<double> c(net.begin(), net.end());

        // Calculate network utilization
        for (int i = 0; i < sz; i++)
        {
            util += c[i] / (c[i] + compute_time[i]);
        }
        cout << util << " is the util \n";

        if (util > 0.99)  // network bandwidth required by the jobs cannot be supported by this bottleneck link capacity
            continue;

        //// Simulating Normal TCP  ////

        // Assuming that TCP convergence to fair allocation in every second
        // Thus one second can be taken as the time quantum (discrete sim)

        vector<int> stats(sz, 0);
        vector<int> w(sz, 0);
        int tempsum = 0;
        for(int i=0; i<sz; i++)
        {
            w[i] = tempsum;
            tempsum += compute_time[i];
        }

        float DELTA = 0.01;

        for (int tick = 0; tick < SIMTICKS; tick++)
        {
            int active_jobs = 0;

            // Count active servers
            for (int j = 0; j < sz; j++)
            {
                if (w[j] == 0 && c[j] > DELTA) 
                {
                    active_jobs++;
                }
            }

            // Update network/compute time remaining
            for (int j = 0; j < sz; j++)
            {
                if (c[j] < DELTA)  // network part of the job is completed, it will perform compute for compute_time[j] time now
                {
                    c[j] = net[j];  
                    w[j] = compute_time[j];
                    stats[j]++;
                }
                else if (w[j] > 0)  // the job is in compute phase
                {
                    w[j]--;
                }
                else
                {
                    c[j] -= 1.0 / active_jobs;
                }
            }
        }

        // Compute average fractions for fair TCP and MLTCP
        int sum = x + sumnet;
        double avg_fraction_fairtcp = 0;
        double avg_fraction_mltcp = 0;

        for (int i = 0; i < sz; i++)
        {
            double statValue = stats[i];
            double fairtcp = (stats[i] * compute_time[i]) / static_cast<double>(SIMTICKS);
            double fraction = static_cast<double>(compute_time[i]) / (net[i] + compute_time[i]);
            double mltcp = static_cast<double>(compute_time[i]) / sum;

            // Output for each job
            cout << i << " : " << statValue << ", " << fairtcp << " out of " << fraction << " whereas MLTCP is "
                 << mltcp << "\n";

            avg_fraction_mltcp += mltcp;
            avg_fraction_fairtcp += fairtcp;
        }

        avg_fraction_mltcp /= sz;
        avg_fraction_fairtcp /= sz;

        // Print average compute utilization
        cout << "Average compute util (for x=" << x << "): " << avg_fraction_fairtcp << ", " << avg_fraction_mltcp
             << "\n";

        cout << "Ratio: " << avg_fraction_fairtcp/avg_fraction_mltcp << "\n";
    }

    return 0;
}
