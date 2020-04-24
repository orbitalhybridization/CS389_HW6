
Cities::permutation_t
ga_search(const Cities& cities,
          unsigned iters,
          unsigned pop_size,
          double mutation_rate,
          unsigned nthread = 1)
{
  //auto best_dist = 1e100 + nthread; // Eliminate silly warning
  auto best_ordering = Cities::permutation_t(cities.size());


  auto best_mutex = std::mutex();
  //std::atomic<unsigned> evaluated = 0;

  auto run_one_thread = [&]() {
    //while (evaluated < iters) {
    auto my_best = baby_search(cities, iters / nthread, pop_size, mutation_rate);

    if (cities.total_path_distance(my_best) < cities.total_path_distance(best_ordering)) {
      auto guard = std::scoped_lock(best_mutex);
      // Repeat check, maybe something changed:
      if (cities.total_path_distance(my_best) < cities.total_path_distance(best_ordering)) {
        best_ordering = my_best;
      }
    }
  };

  std::vector<std::thread> threads;
  for (unsigned i = 0; i < nthread; ++i) {
    threads.push_back(std::thread(run_one_thread));
  }

  for (auto& t : threads) {
    t.join();
  }

  return best_ordering;
}




//////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
  if (argc < 4) {
    std::cerr << "Required arguments: filename for cities, population size, and mutation rate\n";
    return -1;
  }

  const auto cities = Cities(argv[1]);
  const auto pop_size = atoi(argv[2]);
  const auto mut_rate = atof(argv[3]);
  const auto nthread = (argc > 4)? atoi(argv[4]) : 1;
  const auto granularity = (argc > 5)? atoi(argv[5]) : 100;

  constexpr unsigned NUM_ITER = 2'000'000;
  assert(cities.size() > 0 && "Did you actually read the input file successfully?");


//  const auto best_ordering = exhaustive_search(cities);
//  const auto best_ordering = randomized_search(cities, NUM_ITER);
//  const auto best_ordering = threaded_randomized_search(cities, NUM_ITER, nthread);
//  const auto best_ordering = granular_randomized_search(cities, NUM_ITER, nthread, granularity);
  const auto best_ordering = ga_search(cities, NUM_ITER, pop_size, mut_rate, nthread);

  auto out = std::ofstream("shortest.tsv");
  if (!out.is_open()) {
    std::cerr << "Can't open output file to record shortest path!\n";
    return -2;
  }

  out << cities.reorder(best_ordering);

  return pop_size - mut_rate + nthread - granularity; // eliminate silly warning
}


