#ifndef _TIME_PROFILER_H_
#define _TIME_PROFILER_H_

#include <string>
#include <vector>
#include <chrono>


class ProfileManager
{
public:
	static std::vector<std::string> profilers_;
	static std::vector<int> counts_;
	static std::vector<std::chrono::duration<double> > times_;
	static void saveResult(std::string name, std::chrono::duration<double>  time){
		int index = -1;
		for(size_t i =0; i< profilers_.size(); i++){
			if(profilers_[i]==name){
				index=(int)i;
			}
		}
		if(index==-1){
			profilers_.push_back(name);
			counts_.push_back(1);
			times_.push_back(time);
		}else{
			counts_[index]++;
			times_[index]+=time;
		}

	}

	static void printResults(){
		printf("\n--------------------------------------------------------------------\n");
		printf("Name:                        Hz:     Avg Time:        Count:        \n");
		for(size_t i =0; i< profilers_.size(); i++){
			printf("%-24s%8.3f%14.3f%14i\n", profilers_[i].c_str(),counts_[i]/(float)times_[i].count(),times_[i].count()/(float)counts_[i],counts_[i]);
		}
		printf("\n--------------------------------------------------------------------\n");
	}
private:
	ProfileManager();
	~ProfileManager();
	
};

class Profiler{
public:
	std::chrono::time_point<std::chrono::system_clock> start_;
	std::string name_;
	Profiler(std::string name):
	name_(name),
	start_(std::chrono::system_clock::now()){
	}

	~Profiler(){
		ProfileManager::saveResult(name_,std::chrono::system_clock::now()-start_);
	}
};

#define MYPROFILE(x) Profiler profileThis(#x);



#endif