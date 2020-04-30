//baseline

#include "benchmarks.cc" 
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

const int N_REQ = 5000;
const int N_THREADS =5;

std::vector<double> baseline_latencies(int nreq){

    thread_local Cache* cache = new Cache("127.0.0.1","6969");

    //keep in mind cache object, eitan said 

    warm_cache(nreq, cache);
    std::vector<double> output_vec;
    //time for part 3
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int j = 0; j<nreq; j++){

    //get random request
    double work = generate(1, cache); //genorate takes n_req, which would be one at a time 
    output_vec.push_back(work);
    
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    duration*=pow(10,-6); //converts to seconds
    std::cout<<duration<<"<- duration"<<std::endl;

    delete cache;
    return output_vec;
}


// An example from lecture notes
std::pair<double, double> threaded_performance(const int nthreads)
//changed return type to pair from void ~AJizzle with the blizzle
{
    int num = nthreads * N_REQ; //total number of outputs from the threads running baseline_latencies
 
    std::vector<double> output;
    output.reserve(num); //vector of all the performance times across threads

	auto latencies_mutex = std::mutex(); // set up mutexes? muteces? mutecies? muteces? MUTECI?!
	//auto performance_mutex = std::mutex();

	auto run_one_thread = [&]() { //each thread will run baseline_latencies and return the result
		auto out_vec = baseline_latencies(N_REQ);
        auto guard = std::scoped_lock(latencies_mutex); // we're accessing protected data so lock!
		
        output.insert(output.end(), out_vec.begin(),out_vec.end());
	};
    
    std::vector<std::thread> threads; //not quite sure what you meant to do with this so I just put all the "threads" vector references as this
    threads.reserve(nthreads); // for efficiency, allocate space for nthreads    

	for (int i = 0; i < nthreads; ++i) { //add threads to vector
		threads.push_back(std::thread(run_one_thread));
	}

	for (auto& t : threads) { // join threads so they all can finish their work!
		t.join();
	}


    sort(output.begin(),output.end());
    double total;
    for (int i =0; i < num; i++){total+=output[i];} 
    double req_per = num / (total * 0.001); //average of values in vector (converted to seconds)
    //perc is the 95th percentile barrier in the vector
    const int perc = num * 0.95;
    double ret = output[perc]; //ret is the 95th percentile of vec

    std::pair<double, double> output_pair;
    output_pair.first = ret;
    output_pair.second = req_per;
    return output_pair;


}

int main() {
    
    //for (int i = N_THREADS; i <9; i++){
    std::cout<<N_THREADS<< " <- Threadz"<<std::endl;
	auto output = threaded_performance(N_THREADS);
	std::cout<<output.first<<"<- 95th percentile"<< output.second<<"<-req/per"<<std::endl;
//}
	
	return 1;
}
