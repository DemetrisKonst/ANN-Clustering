namespace utils {
  template <typename T>
  std::vector<int, std::vector<Item<T>>> resolveAssignmentConflicts (std::vector<int, std::vector<Item<T>>> items) {

  }


  /* helper method to compute the probabilities of any data point being picked as the next center */
  void compute_probabilities(double* probabilities, double* min_probability, const double* distances, const double& sum_of_distances, const uint32_t& n)
  {
    /* keep a variable for the partial sum */
    double partial_sum = 0.0;

    /* loop to assign a probability to each data point */
    for (int i = 0; i < n; i++)
    {
      /* increment the partial sum */
      partial_sum += distances[i];
      /* assign the probability */
      probabilities[i] = partial_sum / sum_of_distances;
      /* also keep track of the min probability */
      if (probabilities[i] < *min_probability && probabilities[i] != 0)
      {
        *min_probability = probabilities[i];
      }
    }
  }


  /* helper method that returns a random uniformly distributed double in the range [0, 1] */
  double random_uniform_double(const double& min_probability)
  {
    /* find the inverse of the min probability: we need it to make sure that every example has even the slightest chance of being picked */
    int inverse_min_probability = 1.0 / min_probability;

    /* get a random number (RAND_MAX is defined high) */
    int number = rand();

    /* compute the probability and return it */
    double prob = ((number % inverse_min_probability) + 1.0) / inverse_min_probability;
    return prob;
  }


  /* binary search method for double arrays */
  template <typename T>
  T binarySearch(const T sortedArray[], int first, int last, const T& key)
  {
    /* while the left side is smaller than the right */
    while (first <= last)
    {
      /* get the mid value */
      int mid = (first + last) / 2;

      /* if the key is bigger than the mid value */
      if (key > sortedArray[mid])
      {
        /* set the left value to mid + 1 */
        first = mid + 1;
      }
      /* else if it is smaller */
      else if (key < sortedArray[mid])
      {
        /* set right value to mid -1 */
        last = mid - 1;
      }
      /* else we have found the value */
      else
      {
        /* return the index */
        return mid;
      }
    }
    /* return the next bigger value, if the same was not found */
    return first;
  }


  /* helper methods that selects an index from a probability matrix */
  uint32_t pick_index_from_probability(const double probabilities[], const double& random_probability, const uint16_t& n)
  {
    /* check the corner case to avoid computations */
    if (random_probability == 1)
    {
      return n - 1;
    }

    double sum = 0.0;
    for (int i = 0; i < n; i++)
    {
      sum += probabilities[i];
    }

    /* return the value found */
    return binarySearch(probabilities, 0, n - 1, random_probability);
  }

}
