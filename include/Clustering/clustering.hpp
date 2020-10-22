#ifndef _CLUSTERING
#define _CLUSTERING

#define BASELINE_K

#include <iostream>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <random>

#include "../interfaces/clustering_interface.h"
#include "../metrics/metrics.hpp"


/* namespace used to implement clustering algorithms */
namespace clustering
{
  /* struct used to denote a Centroid */
  template <typename T>
  struct Centroid
  {
    uint16_t dimension;
    T* components;
    std::vector<T**>* vectors_in_cluster;


    /* constructor */
    Centroid(const T* x, const uint16_t& d): dimension(d)
    {
      components = new T[dimension];
      std::memcpy(components, x, sizeof(T) * dimension);
      vectors_in_cluster = new std::vector<T**>;
    }


    /* destructor */
    ~Centroid(void)
    {
      delete[] components;
      delete vectors_in_cluster;
    }


    /* method to compute distance between centroid and a point */
    double distance_to_point(T* p, const metrics::Metric& metric=metrics::MANHATTAN)
    {
      /* variable that will contain the distance of the 2 points */
      double distance = 0;

      /* see which metric to use for the distance */
      switch (metric)
      {
        case metrics::MANHATTAN:
        {
          distance = metrics::ManhattanDistanceTmp(components, p, dimension);
          break;
        }
        case metrics::EUCLIDEAN:
        {
          distance = metrics::EuclideanDistance(components, p, dimension);
          break;
        }
        case metrics::MAX:
        {
          distance = metrics::MaxDistance(components, p, dimension);
          break;
        }
        case metrics::NON_ZERO:
        {
          distance = metrics::nonZeroDistance(components, p, dimension);
          break;
        }
        default:
        {
          std::cout << "Unknown metric in Centroid::distance_to_point().\n";
          break;
        }
      }

      /* return the result */
      return distance;
    }


    /* method to print the components of a centroid */
    void print_centroid(void)
    {
      for (int i = 0; i < dimension; i++)
      {
        std::cout << +components[i] << " ";
      }
      std::cout << std::endl;
    }

  };


  /* class used for the implementation of the Clustering algorithms */
  template <typename T>
  class Clustering
  {

  private:
    uint16_t K;
    uint8_t LSH_L = 3;
    uint8_t LSH_k = 4;
    uint32_t HC_M = 10;
    uint8_t HC_k = 3;
    uint16_t HC_probes = 2;
    Centroid<T>** centroids;


    /* helper method that returns a random uniformly distributed double in the range [0, 1] */
    double _random_uniform_double(const double& min_probability)
    {
      /* find the inverse of the min probability: we need it to make sure that every example has even the slightest chance of being picked */
      int inverse_min_probability = 1.0 / min_probability;

      /* get a random number (RAND_MAX is defined high) */
      int number = rand();

      /* compute the probability and return it */
      double prob = ((number % inverse_min_probability) + 1.0) / inverse_min_probability;
      return prob;
    }


    /* helper methods that selects an index from a probability matrix */
    uint32_t _pick_index_from_probability(const double probabilities[], const double& random_probability, const uint16_t& n)
    {
      /* check the corner case to avoid computations */
      if (random_probability == 1)
      {
        return n - 1;
      }

      /* use variables to locate the wanted probability */
      double current_probability = probabilities[0];
      uint16_t current_index = 0;

      /* perform a loop to increment the sum of probabilities until we reach the wanted value */
      while (current_probability < random_probability && current_index < n)
      {
        current_index++;
        current_probability += probabilities[current_index];
      }

      /* return the value found */
      return current_index;
    }


    /* helper method to compute the min distances of each data point to any centroid */
    void _compute_distances(double* distances, double* sum_of_distances, const uint16_t& current_centroids, const interface::Dataset& data, const uint32_t& n)
    {
      /* loop through the dataset to compute the squared distance to the centroids */
      for (int i = 0; i < n; i++)
      {
        /* variable to store the min distance found */
        double min_distance = centroids[0]->distance_to_point(data.images[i]);

        /* compare the distance to each available centroid */
        for (int c = 1; c < current_centroids; c++)
        {
          double distance = centroids[c]->distance_to_point(data.images[i]);
          if (distance < min_distance)
          {
            min_distance = distance;
          }
        }

        /* store the square of the distance */
        distances[i] = min_distance * min_distance;

        /* add it to the sum of min distances */
        *sum_of_distances += distances[i];
      }
    }


    /* helper method to compute the probabilities of any data point being picked as the next centroid */
    void _compute_probabilities(double* probabilities, double* min_probability, const double* distances, const double& sum_of_distances, const uint32_t& n)
    {
      /* loop to assign a probability to each data point */
      for (int i = 0; i < n; i++)
      {
        probabilities[i] = distances[i] / sum_of_distances;
        if (probabilities[i] < *min_probability && probabilities[i] != 0)
        {
          *min_probability = probabilities[i];
        }
      }
    }


    /* method that implements intialization++ */
    void _initialize_centroids(const interface::Dataset& data)
    {
      /* get the number of training examples */
      uint32_t n = data.number_of_images;
      /* get the dimension of each image */
      uint16_t d = data.rows_per_image * data.columns_per_image;

      /* pick an example at random */
      srand(time(NULL));
      uint32_t random_example = rand() % n;

      /* initialize it to be the first centroid */
      centroids[0] = new Centroid<T>(data.images[random_example], d);
      uint16_t current_centroids = 1;


      /* repeat the process until K centroids have been created */
      while (current_centroids < K)
      {
        /* intialize an array to store all the distances squared of each point to the centroid */
        double distances[n] = {0.0};
        /* store the sum of all the distances in a variable (it will not overflow, I have checked it) */
        double sum_of_distances = 0.0;

        /* compute the min distances D[i] */
        _compute_distances(distances, &sum_of_distances, current_centroids, data, n);

        /* intialize an array to store all the probabilities of an example being picked as the next centroid */
        double probabilities[n] = {0.0};
        /* store the min probability in a variable */
        double min_probability = 1.0;

        /* compute the probability of each data point being picked as the next centroid */
        _compute_probabilities(probabilities, &min_probability, distances, sum_of_distances, n);

        /* create a random uniform (X ~ U(-, 1.0 / min_probability)) double that will be used to pick the new centroid */
        double random_probability = _random_uniform_double(min_probability);

        /* find the next centroid from the random probability, and create it */
        uint32_t next_centroid = _pick_index_from_probability(probabilities, random_probability, n);
        centroids[current_centroids] = new Centroid<T>(data.images[next_centroid], d);

        /* increment the number of centroids */
        current_centroids++;
      }
    }

  public:

    /* constructor used to build the object */
    Clustering(const interface::input::clustering::clustering_config& configuration, const interface::Dataset& data):
    K(configuration.clusters_K), LSH_L(configuration.LSH_L), LSH_k(configuration.LSH_k),
    HC_M(configuration.HC_M), HC_k(configuration.HC_k), HC_probes(configuration.HC_probes)
    {
      /* create K pointers to centroids (basically K centroids) */
      centroids = new Centroid<T>*[K];
      /* initialize them with initialization++ (k-means++) */
      _initialize_centroids(data);
    }


    /* destructor called when the object is destroyed */
    ~Clustering(void)
    {
      /* delete each centroid */
      for (int i = 0; i < K; i++)
      {
        delete centroids[i];
      }
      /* delete matrix containing them */
      delete[] centroids;
    }


  };
}




#endif
