//baseline

#include "benchmarks.cc" 
#include <fstream>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>

const int N_REQ = 318;
const int N_THREADS = 3;

std::vector<double> baseline_latencies(int nreq){

    thread_local Cache* cache = new Cache("127.0.0.1","6969");

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

std::pair<double,double> baseline_performance(int num){

    //if we change the vector type for baseline_lat... make sure to change this one too :)
    std::vector<double> vec = baseline_latencies(num);
    sort(vec.begin(),vec.end());
    double total;
    for (int i =0; i < num; i++){total+=vec[i];} 
    double req_per = (total * 0.001) / num; //average of values in vector
    //perc is the 95th percentile barrier in the vector
    const int perc = num * 0.95;
    double ret = vec[perc]; //ret is the 95th percentile of vec

    std::pair<double, double> output;
    output.first = ret;
    output.second = req_per;
    
    return output;

}

// An example from lecture notes
std::pair<double, double> threaded_performance(const int nthreads)
//changed return type to pair from void ~AJizzle with the blizzle
{
	
	std::vector<std::thread> thread_reqper_results; //not quite sure what you meant to do with this so I just put all the "threads" vector references as this
	thread_reqper_results.reserve(nthreads); // for efficiency, allocate space for nthreads
	std::vector<double> thread_percentile_results;
    std::vector<double> thread_nreq_results; //IArthur created to match what you 


	auto latencies_mutex = std::mutex(); // set up mutexes? muteces? mutecies? muteces? MUTECI?!
	auto performance_mutex = std::mutex();
 

	auto run_one_thread = [&]() { //each thread will run baseline_latencies and return the result
		
		auto guard = std::scoped_lock(performance_mutex); // we're accessing protected data so lock!
		
		const std::pair<double,double> out_pair = baseline_performance(N_REQ);

		thread_nreq_results.push_back(out_pair.second);
		thread_percentile_results.push_back(out_pair.first); //i think u meant to declare this as a vector of doubles instead of threads...i"ll change it 

	};

	for (int i = 0; i < nthreads; ++i) { //add threads to vector
		thread_reqper_results.push_back(std::thread(run_one_thread));
	}

	for (auto& t : thread_reqper_results) { // join threads so they all can finish their work!
		t.join();
	}


	// Find 95th percentile and total "throughput" (req/sec I think)
    sort(thread_nreq_results.begin(),thread_nreq_results.end());
	sort(thread_percentile_results.begin(),thread_percentile_results.end());

    double total;

    for (int i =0; i < N_REQ; i++){total+=thread_nreq_results[i];}  

    const double req_per = (total * 0.001) / N_REQ;

    //perc is the 95th percentile barrier in the vector
    const int perc = N_REQ * 0.95;

    double ret = thread_percentile_results[perc]; //ret is the 95th percentile of vec

    std::pair<double, double> output;
    output.first = ret;
    output.second = req_per;
    
    return output;

}

int main() {

	auto output = threaded_performance(N_THREADS);
	std::cout<<output.first<<"<- 95th percentile"<< output.second<<"<-req/per"<<std::endl;
	
	return 1;
}
