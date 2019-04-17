#include "time_profiler.hpp"

//These variables need to be defined in the cpp

std::vector<std::string> ProfileManager::profilers_;
std::vector<int> ProfileManager::counts_;
std::vector<std::chrono::duration<double> > ProfileManager::times_;