#ifndef _CLUSTERING
#define _CLUSTERING

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
  /* struct used to denote a ClusterCenter */
  template <typename T>
  struct ClusterCenter
  {
    uint16_t dimension;
    T* components;
    std::vector<uint32_t>* vectors_in_cluster;


    /* constructor */
    ClusterCenter(const T* x, const uint16_t& d): dimension(d)
    {
      components = new T[dimension];
      std::memcpy(components, x, sizeof(T) * dimension);
      vectors_in_cluster = new std::vector<uint32_t>;
    }


    /* destructor */
    ~ClusterCenter(void)
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
          std::cout << "Unknown metric in ClusterCenter::distance_to_point().\n";
          break;
        }
      }

      /* return the result */
      return distance;
    }


    /* method used to clear a cluster center (e.g. remove all the items it currenly has) */
    void clear_cluster_center(void)
    {
      vectors_in_cluster->clear();
    }


    /* method used to add a data point to the cluster of cluster center */
    void add_to_cluster(uint32_t p)
    {
      vectors_in_cluster->push_back(p);
    }


    /* method used to compute the new cluster center from the data points in the cluter; returns true if the center changes; else false */
    void compute_new_center_from_data(bool* center_changed, const interface::Dataset& data)
    {
      /* create the new center */
      T* new_center = new T[dimension];

      /* loop to compute each component of the new center */
      for (int d = 0; d < dimension; d++)
      {
        /* sum of d component of data points in the cluster */
        double sum = 0.0;

        /* iterate through the data points in the same cluster */
        for (int i = 0; i < vectors_in_cluster->size(); i++)
        {
          /* add the d-th component of the i-th data point */
          uint32_t index = (*vectors_in_cluster)[i];
          sum += data.images[index][d];
        }

        /* compute the new value and assign it to the center */
        double new_value = ((double) sum) / vectors_in_cluster->size();
        new_center[d] = new_value;

        /* check whether a component has changed its value */
        if (new_center[d] != components[d])
        {
          *center_changed = true;
        }
      }

      /* free the memory of the previous components */
      delete[] components;

      /* assign the new value */
      components = new_center;
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


    /* method used to compute the silhouette of each element inside the cluster */

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
    ClusterCenter<T>** centers;
    std::vector<double>* silhouettes;


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


    /* helper method to compute the min distances of each data point to any center */
    void _compute_distances(double* distances, double* sum_of_distances, const uint16_t& current_centers, const interface::Dataset& data, const uint32_t& n)
    {
      /* loop through the dataset to compute the squared distance to the centers */
      for (int i = 0; i < n; i++)
      {
        /* variable to store the min distance found */
        double min_distance = centers[0]->distance_to_point(data.images[i]);

        /* compare the distance to each available center */
        for (int c = 1; c < current_centers; c++)
        {
          double distance = centers[c]->distance_to_point(data.images[i]);
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


    /* helper method to compute the probabilities of any data point being picked as the next center */
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
    void _initialize_centers(const interface::Dataset& data)
    {
      /* get the number of training examples */
      uint32_t n = data.number_of_images;
      /* get the dimension of the training examples */
      uint16_t d = data.rows_per_image * data.columns_per_image;

      /* pick an example at random */
      srand(time(NULL));
      uint32_t random_example = rand() % n;

      /* initialize it to be the first center */
      centers[0] = new ClusterCenter<T>(data.images[random_example], d);
      uint16_t current_centers = 1;


      /* repeat the process until K centers have been created */
      while (current_centers < K)
      {
        /* intialize an array to store all the distances squared of each point to the center */
        double distances[n] = {0.0};
        /* store the sum of all the distances in a variable (it will not overflow, I have checked it) */
        double sum_of_distances = 0.0;

        /* compute the min distances D[i] */
        _compute_distances(distances, &sum_of_distances, current_centers, data, n);

        /* intialize an array to store all the probabilities of an example being picked as the next center */
        double probabilities[n] = {0.0};
        /* store the min probability in a variable */
        double min_probability = 1.0;

        /* compute the probability of each data point being picked as the next center */
        _compute_probabilities(probabilities, &min_probability, distances, sum_of_distances, n);

        /* create a random uniform (X ~ U(-, 1.0 / min_probability)) double that will be used to pick the new center */
        double random_probability = _random_uniform_double(min_probability);

        /* find the next center from the random probability, and create it */
        uint32_t next_center = _pick_index_from_probability(probabilities, random_probability, n);
        centers[current_centers] = new ClusterCenter<T>(data.images[next_center], d);

        /* increment the number of centers */
        current_centers++;
      }
    }


    /* method that clears the cluster centers so that they can get new data points assigned */
    void _clear_centers(void)
    {
      /* do this for every cluster center */
      for (int i = 0; i < K; i++)
      {
        centers[i]->clear_cluster_center();
      }
    }


    /* helper method used to find the closest cluster center to a data point */
    uint16_t _find_closest_center(T* p)
    {
      /* find the distance to first cluster center */
      uint16_t closest_center = 0;
      double min_distance = centers[0]->distance_to_point(p);

      /* compare with the rest cluster centers */
      for (int i = 1; i < K; i++)
      {
        double distance = centers[i]->distance_to_point(p);
        if (distance < min_distance)
        {
          closest_center = i;
          min_distance = distance;
        }
      }

      /* return the result */
      return closest_center;
    }


    /* helper method used to find the closest cluster center to a data point */
    uint16_t _find_second_closest_center(T* p)
    {
      /* variables to keep track of closest centers and their distances */
      uint16_t closest_center = -1;
      uint16_t second_closest_center = -1;
      double min_distance = -1;
      double second_min_distance = -1;

      /* check which of the first 2 clusters are closer */
      if (centers[0]->distance_to_point(p) < centers[1]->distance_to_point(p))
      {
        second_closest_center = 1;
        second_min_distance = centers[1]->distance_to_point(p);
        closest_center = 0;
        min_distance = centers[0]->distance_to_point(p);
      }
      else
      {
        second_closest_center = 0;
        second_min_distance = centers[0]->distance_to_point(p);
        closest_center = 1;
        min_distance = centers[1]->distance_to_point(p);
      }

      /* compare with the rest centers */
      for (int c = 2; c < K; c++)
      {
        /* compute the distance to the current center */
        double distance = centers[c]->distance_to_point(p);

        /* check if the new center is closest or second closest */
        if (distance < min_distance)
        {
          second_closest_center = closest_center;
          closest_center = c;
          second_min_distance = min_distance;
          min_distance = distance;
        }
        else if (distance < second_min_distance)
        {
          second_closest_center = c;
          second_min_distance = distance;
        }
      }

      /* return the second closest center */
      return second_closest_center;
    }


    /* method that implements Lloyds Algorithm for Clustering */
    void _Lloyd_Clustering(const interface::Dataset& data)
    {
      /* initialize some useful variables */
      uint32_t n = data.number_of_images;
      uint32_t dimension = data.rows_per_image * data.columns_per_image;

      /* keep a flag that will be used to determine whether a change was made to any cluster center */
      bool center_changed = true;

      /* keep iterating while there are changed made in the cluster centers */
      while (center_changed)
      {
        /* set the flag to false, so that it gets set to true only if a change is made */
        center_changed = false;

        /* first remove the "assignment" of data points in the cluster centers */
        _clear_centers();

        /* loop through data points to find the closest cluster senter */
        for (int i = 0; i < n; i++)
        {
          /* find the closest cluster center for training example i */
          uint16_t closest_center = _find_closest_center(data.images[i]);

          /* add data point to the cluster */
          centers[closest_center]->add_to_cluster(i);
        }

        /* compute the new cluster centers */
        for (int c = 0; c < K; c++)
        {
          centers[c]->compute_new_center_from_data(&center_changed, data);
        }
      }
    }


    /* method that implements Reverse Assignment with LSH Algorithm for Clustering */
    void _LSH_Clustering(const interface::Dataset& data)
    {

    }


    /* method that implements Reverse Assignment with Hypercube Algorithm for Clustering */
    void _HC_Clustering(const interface::Dataset& data)
    {

    }


    /* method to compute the silhouette of each data point */
    void _compute_silhouettes(double* s, const interface::Dataset& data, const uint32_t& n, const uint16_t& dimension)
    {
      /* array to store the average distance of each point to the points in the same cluster */
      double* a = new double[n];
      /* array to store the average distance of each point to the points in the next closest cluster */
      double* b = new double[n];
      /* array used to store the second closest cluster to each datapoint */
      uint16_t* second_closest_cluster = new uint16_t[n];

      /* compute the second closest cluster for each data point */
      for (int i = 0; i < n; i++)
      {
        second_closest_cluster[i] = _find_second_closest_center(data.images[i]);
      }



      /* free up used memory */
      delete[] a;
      delete[] b;
      delete[] second_closest_cluster;
    }


  public:

    /* constructor used to build the object */
    Clustering(const interface::input::clustering::clustering_config& configuration, const interface::Dataset& data):
    K(configuration.clusters_K), LSH_L(configuration.LSH_L), LSH_k(configuration.LSH_k),
    HC_M(configuration.HC_M), HC_k(configuration.HC_k), HC_probes(configuration.HC_probes)
    {
      /* create a vector that will be used to store the silhouettes */
      silhouettes = new std::vector<double>;
      /* create K pointers to Cluster Centers (basically K centers) */
      centers = new ClusterCenter<T>*[K];
      /* initialize them with initialization++ (k-means++) */
      _initialize_centers(data);
    }


    /* destructor called when the object is destroyed */
    ~Clustering(void)
    {
      /* free up the vector with the silhouettes for each data point */
      silhouettes->clear();
      delete silhouettes;

      /* delete each cluster center */
      for (int i = 0; i < K; i++)
      {
        delete centers[i];
      }
      /* delete matrix containing them */
      delete[] centers;
    }


    /* method to call the correct clustering algorithm */
    void perform_clustering(const interface::Dataset& data, const std::string& method)
    {
      /* determine which algorithm to use */
      if (method == "Classic")
      {
        _Lloyd_Clustering(data);
      }
      else if (method == "LSH")
      {
        _LSH_Clustering(data);
      }
      else if (method == "Hypercube")
      {
        _HC_Clustering(data);
      }
      else
      {
        std::cout << "ELOUSA\n";
        exit(EXIT_FAILURE);
      }
    }


    /* method that returns the Cluster Centers */
    ClusterCenter<T>** get_centers(void)
    {
      return centers;
    }


    /* method used to compute silhouette of a clustering */
    double compute_average_silhouette(const interface::Dataset& data)
    {
      /* initialize some useful variables */
      uint32_t n = data.number_of_images;
      uint32_t dimension = data.rows_per_image * data.columns_per_image;

      /* create an array to store the silhouette value of each data point, and after transfer it to the silhouettes vector */
      double* s = new double[n];

      /* call internal method to compute the silhouette of each data point */
      _compute_silhouettes(s, data, n, dimension);

      /* store the silhouettes in the vector of the class */
      for (int i = 0; i < n; i++)
      {
        silhouettes->push_back(s[i]);
      }

      /* compute the sum of all the silhouettes in order to later compute the average */
      double sum = 0.0;
      for (int i = 0; i < n; i++)
      {
        sum += s[i];
      }

      /* compute the average silhouette */
      double average_silhouette = ((double) sum) / n;

      /* free the silhouettes array */
      delete[] silhouettes;

      /* return the result */
      return average_silhouette;
    }

  };
}




#endif
