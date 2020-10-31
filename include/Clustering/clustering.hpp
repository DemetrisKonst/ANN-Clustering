#ifndef _CLUSTERING
#define _CLUSTERING

#include <iostream>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <functional>
#include <utility>
#include <unordered_map>

#include "../interfaces/clustering_interface.h"
#include "../interfaces/LSH_interface.h"
#include "../metrics/metrics.hpp"
#include "../core/item.hpp"
#include "../utils/cluster.hpp"
#include "../LSH/LSH.hpp"
#include "../Hypercube/Hypercube.hpp"
#include "../utils/ANN.hpp"


/* namespace used to implement clustering algorithms */
namespace clustering
{
  /* struct used to denote a ClusterCenter */
  template <typename T>
  struct ClusterCenter
  {
    uint16_t dimension;
    T* initial_components;
    T* components;
    std::vector<Item<T>*>* vectors_in_cluster;
    double silhouette;


    /* constructor */
    ClusterCenter(const T* x, const uint16_t& d): dimension(d)
    {
      /* initialize the silhouette */
      silhouette = std::numeric_limits<double>::lowest();

      /* initialize an array that will store the initial components assigned to the centroid */
      initial_components = new T[dimension];
      std::memcpy(initial_components, x, sizeof(T) * dimension);

      /* initialize an array that will store the current components of the centroid */
      components = new T[dimension];
      std::memcpy(components, x, sizeof(T) * dimension);

      /* initialize a vector that will contain all the data points (items) inside the cluster */
      vectors_in_cluster = new std::vector<Item<T>*>;
    }


    /* destructor */
    ~ClusterCenter(void)
    {
      /* free allocated memory */
      delete[] initial_components;
      delete[] components;
      delete vectors_in_cluster;
    }


    /* method to compute distance between centroid and a point */
    double distance_to_point(Item<T>* p, const metrics::Metric& metric=metrics::MANHATTAN)
    {
      /* variable that will contain the distance of the 2 points */
      double distance = 0;

      /* see which metric to use for the distance */
      switch (metric)
      {
        case metrics::MANHATTAN:
        {
          distance = metrics::ManhattanDistance(components, p->data, dimension);
          break;
        }
        case metrics::EUCLIDEAN:
        {
          distance = metrics::EuclideanDistance(components, p->data, dimension);
          break;
        }
        case metrics::MAX:
        {
          distance = metrics::MaxDistance(components, p->data, dimension);
          break;
        }
        case metrics::NON_ZERO:
        {
          distance = metrics::nonZeroDistance(components, p->data, dimension);
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
    void add_to_cluster(Item<T>* p)
    {
      vectors_in_cluster->push_back(p);
    }


    /* method used to compute the new cluster center from the data points in the cluster; returns true if the center changes; else false */
    void compute_new_center_from_data(bool* center_changed, const double& tolerance=3000, const metrics::Metric& metric=metrics::MANHATTAN)
    {
      /* create the new center */
      T* new_center = new T[dimension];

      /* loop to compute each component of the new center */
      for (int d = 0; d < dimension; d++)
      {
        /* vector that will contain the value of d-th dimension for each data point in the cluster */
        std::vector<T> d_dimension;

        /* iterate through the data points in the same cluster */
        for (int i = 0; i < vectors_in_cluster->size(); i++)
        {
          /* add the d-th component of the i-th data point */
          Item<T>* item = (*vectors_in_cluster)[i];
          d_dimension.push_back(item->data[d]);
        }

        /* compute the new value for this component by finding the median value */
        std::nth_element(d_dimension.begin(), d_dimension.begin() + d_dimension.size() / 2, d_dimension.end());
        T new_value = d_dimension[d_dimension.size() / 2];
        new_center[d] = new_value;

      }

      /* compute the distance of the new centroid to the old one */
      double distance = 0;
      /* see which metric to use for the distance */
      switch (metric)
      {
        case metrics::MANHATTAN:
        {
          distance = metrics::ManhattanDistance(components, new_center, dimension);
          break;
        }
        case metrics::EUCLIDEAN:
        {
          distance = metrics::EuclideanDistance(components, new_center, dimension);
          break;
        }
        case metrics::MAX:
        {
          distance = metrics::MaxDistance(components, new_center, dimension);
          break;
        }
        case metrics::NON_ZERO:
        {
          distance = metrics::nonZeroDistance(components, new_center, dimension);
          break;
        }
        default:
        {
          std::cout << "Unknown metric in ClusterCenter::compute_new_center_from_data().\n";
          break;
        }
      }


      /* if the new centroid differs by only a little, consider it as unchanged; else change it */
      if (distance > tolerance)
      {
        *center_changed = true;
      }

      /* free the memory of the previous components */
      delete[] components;

      /* assign the new value */
      components = new_center;
    }


    /* method that resets a cluster center back to its original state */
    void reset_cluster_center(void)
    {
      /* free the memory for the current components */
      delete[] components;

      /* allocate again memory to reinitialize the components */
      components = new T[dimension];
      std::memcpy(components, initial_components, sizeof(T) * dimension);

      /* remove the items that were inside the cluster */
      clear_cluster_center();

      /* reset the silhouette */
      silhouette = std::numeric_limits<double>::lowest();
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


    /* method used to compute the average distance of each element inside the cluster */
    void compute_average_distance_of_objects_in_cluster(double* a, const uint16_t& dimension, const metrics::Metric& metric=metrics::MANHATTAN)
    {
      /* number of elements inside the cluster */
      uint32_t number_of_elements_in_cluster = vectors_in_cluster->size();

      /* for each item inside the cluster */
      for (int i = 0; i < number_of_elements_in_cluster; i++)
      {
        /* get the Item as a variable to facilitate coding */
        Item<T>* item = (*vectors_in_cluster)[i];
        /* get the index of the item */
        uint32_t index = item->id;

        /* compute the total distance of the item in the cluster */
        double total_distance = 0.0;

        /* for each other item in the cluster, add the distance to it in the total distance */
        for (int j = 0; j < number_of_elements_in_cluster; j++)
        {
          /* get the second Item as a variable */
          Item<T>* item2 = (*vectors_in_cluster)[j];

          /* get the distance that will be computed */
          double distance = 0.0;

          /* distinguish each type of distance and compute it */
          switch(metric)
          {
            case metrics::MANHATTAN:
            {
              distance = metrics::ManhattanDistance(item->data, item2->data, dimension);
              break;
            }
            case metrics::EUCLIDEAN:
            {
              distance = metrics::EuclideanDistance(item->data, item2->data, dimension);
              break;
            }
            case metrics::MAX:
            {
              distance = metrics::MaxDistance(item->data, item2->data, dimension);
              break;
            }
            case metrics::NON_ZERO:
            {
              distance = metrics::nonZeroDistance(item->data, item2->data, dimension);
              break;
            }
            default:
            {
              std::cout << "Unknown metric in ClusterCenter::compute_average_distance_of_objects_in_cluster().\n";
              break;
            }
          }

          /* add the distance to the total distance */
          total_distance += distance;
        }

        /* compute the a value of the specific data point and store it in the array */
        double a_value = ((double) total_distance) / (number_of_elements_in_cluster - 1);
        a[index] = a_value;
      }
    }


    /* method used to compute the average distance of each element inside the cluster to a point outside of it */
    double compute_average_distance_of_point_to_objects_in_cluster(Item<T>* p, const uint16_t& dimension, const metrics::Metric& metric=metrics::MANHATTAN)
    {
      /* get the number of elements in the cluster */
      uint32_t number_of_elements_in_cluster = vectors_in_cluster->size();

      /* store the total distance inside a variable */
      double total_distance = 0.0;

      /* for each data point inside the cluster */
      for (int i = 0; i < number_of_elements_in_cluster; i++)
      {
        /* get the Item as a variable */
        Item<T>* item = (*vectors_in_cluster)[i];

        /* get the distance that will be computed */
        double distance = 0.0;

        /* distinguish each type of distance and compute it */
        switch (metric)
        {
          case metrics::MANHATTAN:
          {
            distance = metrics::ManhattanDistance(p->data, item->data, dimension);
            break;
          }
          case metrics::EUCLIDEAN:
          {
            distance = metrics::EuclideanDistance(p->data, item->data, dimension);
            break;
          }
          case metrics::MAX:
          {
            distance = metrics::MaxDistance(p->data, item->data, dimension);
            break;
          }
          case metrics::NON_ZERO:
          {
            distance = metrics::nonZeroDistance(p->data, item->data, dimension);
            break;
          }
          default:
          {
            std::cout << "Unknown metric in ClusterCenter::compute_average_distance_of_objects_in_cluster().\n";
            break;
          }

          /* add the distance to the total distance */
          total_distance += distance;
        }

        /* add the distance to the total distance */
        total_distance += distance;
      }

      /* compute the b value for the specific item and return */
      double b_value = ((double) total_distance) / number_of_elements_in_cluster;
      return b_value;
    }


    /* method to compute the overall silhouette of a cluster */
    void compute_cluster_silhouette(double* s)
    {
      /* sum of all the silhouettes of objects in the cluster */
      double total_silhouette = 0.0;

      /* number of elements inside the cluster */
      uint32_t number_of_elements_in_cluster = vectors_in_cluster->size();

      /* for each item inside the cluster */
      for (int i = 0; i < number_of_elements_in_cluster; i++)
      {
        /* get the Item as a variable to facilitate coding */
        Item<T>* item = (*vectors_in_cluster)[i];
        /* get the index of the item */
        uint32_t index = item->id;

        /* add the silhouette of this item to the total silhouette */
        total_silhouette += s[index];

      }

      /* compute the silhouette of the cluster and assign it */
      double cluster_silhouette = ((double) total_silhouette) / number_of_elements_in_cluster;
      silhouette = cluster_silhouette;
    }


    /* getter method to get the silhouette of a cluster */
    double get_silhouette(void)
    {
      return silhouette;
    }


    /* getter method to get the size of the cluster */
    int get_cluster_size(void)
    {
      return vectors_in_cluster->size();
    }


    /* getter method that returns the components of the cluster center */
    T* get_components(void)
    {
      return components;
    }


    /* getter method that returns the vectors inside the cluster */
    std::vector<Item<T>*>* get_vectors_inside_cluster(void)
    {
      return vectors_in_cluster;
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
    double radius = 0.0;
    double silhouette = 0.0;
    ClusterCenter<T>** centers;
    std::vector<double>* silhouettes;
    LSH<T>* lsh;
    Hypercube<T>* hypercube;


    /* helper method to compute the min distances of each data point to any center */
    void _compute_distances(double* distances, double* sum_of_distances, const uint16_t& current_centers, const interface::Data<T>& data)
    {
      /* loop through the dataset to compute the squared distance to the centers */
      for (int i = 0; i < data.n; i++)
      {
        /* variable to store the min distance found */
        double min_distance = centers[0]->distance_to_point(data.items[i]);

        /* compare the distance to each available center */
        for (int c = 1; c < current_centers; c++)
        {
          double distance = centers[c]->distance_to_point(data.items[i]);
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


    /* method that implements intialization++ */
    void _initialize_centers(const interface::Data<T>& data)
    {
      /* pick an example at random */
      srand(time(NULL));
      uint32_t random_example = rand() % data.n;

      /* initialize it to be the first center */
      centers[0] = new ClusterCenter<T>(data.items[random_example]->data, data.dimension);
      uint16_t current_centers = 1;


      /* repeat the process until K centers have been created */
      while (current_centers < K)
      {
        /* intialize an array to store all the distances squared of each point to the center */
        double distances[data.n] = {0.0};
        /* store the sum of all the distances in a variable (it will not overflow, I have checked it) */
        double sum_of_distances = 0.0;

        /* compute the min distances D[i] for each data point i */
        _compute_distances(distances, &sum_of_distances, current_centers, data);

        /* intialize an array to store all the probabilities of an example being picked as the next center */
        double probabilities[data.n] = {0.0};
        /* store the min probability in a variable */
        double min_probability = 1.0;

        /* compute the probability of each data point being picked as the next center */
        utils::compute_probabilities(probabilities, &min_probability, distances, sum_of_distances, data.n);

        /* create a random uniform (X ~ U(-, 1.0 / min_probability)) double that will be used to pick the new center */
        double random_probability = utils::random_uniform_double(min_probability);

        /* find the next center from the random probability, and create it */
        uint32_t next_center = utils::pick_index_from_probability(probabilities, random_probability, data.n);
        centers[current_centers] = new ClusterCenter<T>(data.items[next_center]->data, data.dimension);

        /* increment the number of centers */
        current_centers++;
      }
    }


    /* method used to initialize useful data structures (LSH or Hypercube) */
    void _initialize_data_structures(const interface::Data<T>& data, const std::string& method)
    {
      /* check which object to initialize, depending on the method desired */
      if (method == "LSH")
      {
        /* check if this is not the first time initializing an LSH object */
        if (lsh != NULL)
        {
          delete lsh;
        }

        /* create an LSH input object */
        interface::input::LSH::LSHInput lsh_input;
        /* assign the values that were given in the configuration file */
        lsh_input.k = LSH_k;
        lsh_input.L = LSH_L;
        radius = lsh_input.R / 2;

        /* compute the window size for LSH */
        double average_item_distance = utils::averageDistance(0.05, data.items, data.n, data.dimension);
        uint32_t window_constant = 1;
        uint32_t window_size = (int) (window_constant * average_item_distance);
        /* initialize the LSH object with a specific window size */
        lsh = new LSH<T>(lsh_input, data, window_size);
      }
      else if (method == "Hypercube")
      {
        /* check if this is not the first time initializing a Hypercube object */
        if (hypercube != NULL)
        {
          delete hypercube;
        }

        /* create an Hypercube input object */
        interface::input::HC::HCInput hc_input;
        /* assign the values that were given in the configuration file */
        hc_input.k = HC_k;
        hc_input.M = HC_M;
        hc_input.probes = HC_probes;
        radius = hc_input.R / 2;

        /* compute the window size for Hypercube */
        double average_item_distance = utils::averageDistance(0.05, data.items, data.n, data.dimension);
        uint32_t window_constant = 1;
        uint32_t window_size = (int) (window_constant * average_item_distance);
        std::pair<double, double> mean_dev = utils::calculateMeanDeviation(1, data.items, data.n, data.dimension, HC_k, window_size);
        uint32_t f_min = floor(mean_dev.first - mean_dev.second);
        uint32_t f_max = floor(mean_dev.first + mean_dev.second);
        /* initialize the Hypercube object */
        hypercube = new Hypercube<T>(hc_input, data, window_size, std::make_pair(f_min, f_max));
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


    /* method that unmarks all data points */
    void _unmark_data_points(const interface::Data<T>& data)
    {
      /* for each data point */
      for (int i = 0; i < data.n; i++)
      {
        /* get the item */
        Item<T>* item = data.items[i];

        /* set it to unmarked */
        item->staged = false;
        item->marked = false;
      }
    }


    /* helper method used to find the closest cluster center to a data point */
    uint16_t _find_closest_center(Item<T>* p)
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
    uint16_t _find_second_closest_center(Item<T>* p)
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
    void _Lloyd_Assignment(const interface::Data<T>& data)
    {
      /* loop through data points to find the closest cluster senter */
      for (int i = 0; i < data.n; i++)
      {
        /* get the item */
        Item<T>* item = data.items[i];

        /* if it is not marked, assign it to a cluster */
        if (!item->marked)
        {
          /* find the closest cluster center for training example i */
          uint16_t closest_center = _find_closest_center(item);

          /* add data point to the cluster */
          centers[closest_center]->add_to_cluster(item);

          /* mark it */
          item->marked = true;
        }
      }
    }


    /* method that implements Reverse Assignment with LSH or HC Algorithm for Clustering */
    void _Reverse_Assignment(const interface::Data<T>& data, const std::string& method, const uint16_t& max_iterations=3)
    {
      /* define some variables used for the stopping conditions */
      uint16_t iterations = 0;
      uint16_t balls_changed = 0;

      /* keep iterating (doubling the radius and performing range search) until stopping conditions are met */
      do
      {
        /* create a vector to temporarily store the assigned cluster for each data point in order to resolve conflicts */
        std::unordered_map<uint32_t, std::pair<uint16_t, Item<T>*>> assigned_clusters;

        /* reset the number *balls* that got points */
        balls_changed = 0;

        /* for each cluster */
        for (uint16_t c = 0; c < K; c++)
        {
          /* define a variable to store the result of the Range Search */
          std::vector<std::pair<int, Item<T>*>> ret;

          /* determine which method to perform Range Search with */
          if (method == "LSH")
          {
            /* perform range search */
            ret = lsh->RangeSearch(centers[c]->get_components(), radius);
          }
          else
          {
            /* perform range search */
            ret = hypercube->RangeSearch(centers[c]->get_components(), radius, HC_probes, 15000);
          }

          /* if this ball gets at least one point, increment the counter */
          if (ret.size() > 0)
          {
            balls_changed++;
          }

          /* add the items of the range search to the cluster */
          for (int i = 0; i < ret.size(); i++)
          {
            /* get the item as a variable to make code more readable */
            Item<T>* item = ret[i].second;

            /* if the item is not staged */
            if (!item->staged)
            {
              /* make it staged because it was found by this query search */
              item->staged = true;
              /* create a pair with (cluster, item) in order to assign it to the cluster */
              std::pair<uint16_t, Item<T>*> pair = std::make_pair(c, item);
              /* assign it temporarily to the current cluster */
              assigned_clusters[item->id] = pair;
            }
            /* else, the item as already staged, so we have to resolve a confict */
            else
            {
              /* get the cluster that was closer so far */
              uint16_t closest_cluster_so_far = assigned_clusters[item->id].first;
              /* compute the distance to it */
              double distance_to_previous_cluster = (double) metrics::ManhattanDistance(item->data, centers[closest_cluster_so_far]->get_components(), data.dimension);
              /* compute the distance to the current cluster */
              double distance_to_current_cluster = (double) metrics::ManhattanDistance(item->data, centers[c]->get_components(), data.dimension);

              /* if the distance to the current cluster is smaller that the distance to the previously closest */
              if (distance_to_current_cluster < distance_to_previous_cluster)
              {
                /* create a pair with (cluster, item) in order to assign it to the cluster */
                std::pair<uint16_t, Item<T>*> pair = std::make_pair(c, item);
                /* assign this item to the current cluster since its closer */
                assigned_clusters[item->id] = pair;
              }
            }

          }

        }

        /* traverse the unordered map, */
        for (auto x : assigned_clusters)
        {
          /* get the value of the specific key in the unordered map */
          std::pair<uint16_t, Item<T>*> pair = x.second;
          /* get the closest cluster found */
          uint16_t closest_cluster = pair.first;
          /* get the item */
          Item<T>* item = pair.second;

          /* unstage the item */
          item->staged = false;
          /* mark the item because it will be added in a cluster */
          item->marked = true;
          /* assign point to its closest luster */
          centers[closest_cluster]->add_to_cluster(item);
        }

        /* double the radius */
        radius *= 2;

        /* increment the number of iterations */
        iterations++;

        /* remove "the assignment" of each data point as the iteration has ended */
        assigned_clusters.clear();

        /* until 80% of balls get new points, and max_iterations have been perfomed at least */
      } while (balls_changed >= K * 0.2 || iterations < max_iterations);


      /* perform Lloyds (brute force) assignment for the rest points that were not assigned */
      _Lloyd_Assignment(data);
    }


    /* method that performs the update step in the clustering procedure */
    void _update_step(bool* center_changed)
    {
      /* compute the new cluster centers */
      for (int c = 0; c < K; c++)
      {
        centers[c]->compute_new_center_from_data(center_changed);
      }
    }


    /* method to compute the silhouette of each data point */
    void _compute_silhouettes(double* s, const interface::Data<T>& data)
    {
      /* array to store the average distance of each point to the points in the same cluster */
      double* a = new double[data.n];
      /* array to store the average distance of each point to the points in the next closest cluster */
      double* b = new double[data.n];
      /* array used to store the second closest cluster to each datapoint */
      uint16_t* second_closest_cluster = new uint16_t[data.n];

      /* compute the second closest cluster for each data point */
      for (int i = 0; i < data.n; i++)
      {
        long int index = data.items[i]->id;
        second_closest_cluster[index] = _find_second_closest_center(data.items[i]);
      }

      /* compute a[i] value for every data point inside a cluster c */
      for (int c = 0; c < K; c++)
      {
        centers[c]->compute_average_distance_of_objects_in_cluster(a, data.dimension);
      }

      /* compute b[i] value for each data point i */
      for (int i = 0; i < data.n; i++)
      {
        long int index = data.items[i]->id;
        b[index] = centers[second_closest_cluster[index]]->compute_average_distance_of_point_to_objects_in_cluster(data.items[index], data.dimension);
      }

      /* and now for each data point i, compute its final silhouette value */
      for (int i = 0; i < data.n; i++)
      {
        long int index = data.items[i]->id;
        s[index] = (b[index] - a[index]) / ((double) std::max(a[index], b[index]));
      }

      /* now that all silhouettes have been computed, compute the silhouettes of clusters */
      for (int c = 0; c < K; c++)
      {
        centers[c]->compute_cluster_silhouette(s);
      }


      /* free up used memory and finish */
      delete[] a;
      delete[] b;
      delete[] second_closest_cluster;
    }


  public:

    /* constructor used to build the object */
    Clustering(const interface::input::clustering::ClusteringConfig& configuration, const interface::Data<T>& data):
    K(configuration.clusters_K), LSH_L(configuration.LSH_L), LSH_k(configuration.LSH_k), HC_M(configuration.HC_M),
    radius(0.0), HC_k(configuration.HC_k), HC_probes(configuration.HC_probes), lsh(NULL), hypercube(NULL)
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

      /* delete the data structures that were used during clustering, if they were used */
      if (lsh != NULL)
      {
        delete lsh;
      }
      if (hypercube != NULL)
      {
        delete hypercube;
      }
    }


    /* method to call the correct clustering algorithm */
    void perform_clustering(const interface::Data<T>& data, const std::string& method, double* duration)
    {
      /* keep a flag that will be used to determine whether a change was made to any cluster center */
      bool center_changed = true;

      /* see if we have to use a Reverse assignment method, and if yes initialize the corresponding object */
      if (method == "LSH" || method == "Hypercube")
      {
        _initialize_data_structures(data, method);
      }

      /* variable used to remeber the start of the execution time */
      auto start = std::chrono::high_resolution_clock::now();

      int iterations = 0;

      /* keep iterating while there are changed made in the cluster centers */
      while (center_changed)
      {
        /* set the flag to false, so that it gets set to true only if a change is made */
        center_changed = false;

        /* first remove the "assignment" of data points in the cluster centers */
        _clear_centers();

        /* unmark all data point */
        _unmark_data_points(data);


        /* determine which algorithm to use in the assignment step */
        if (method == "Classic")
        {
          _Lloyd_Assignment(data);
        }
        else
        {
          _Reverse_Assignment(data, method);
        }

        // /* print the centroids to see that they look like */
        // for (int c = 0; c < K; c++)
        // {
        //   centers[c]->print_centroid();
        //   interface::output::printImage(centers[c]->get_components(), 28, 28);
        // }

        /* now the update step: find new centroids */
        _update_step(&center_changed);

        iterations++;
        // std::cout << "iterations = " << iterations << '\n';
      }

      /* variable used to mark the end of execution */
      auto end = std::chrono::high_resolution_clock::now();

      /* compute the time it took for the clustering */
      *duration = (std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / ((double) 1000000);
    }


    /* method used to compute silhouette of a clustering */
    double compute_average_silhouette(const interface::Data<T>& data)
    {
      /* create an array to store the silhouette value of each data point, and after transfer it to the silhouettes vector */
      double* s = new double[data.n];

      /* call internal method to compute the silhouette of each data point */
      _compute_silhouettes(s, data);

      /* store the silhouettes in the vector of the class */
      for (int i = 0; i < data.n; i++)
      {
        silhouettes->push_back(s[i]);
      }

      /* compute the sum of all the silhouettes in order to later compute the average */
      double sum = 0.0;
      for (int i = 0; i < data.n; i++)
      {
        sum += s[i];
      }

      /* compute the average silhouette */
      double average_silhouette = ((double) sum) / data.n;
      silhouette = average_silhouette;

      /* free the silhouettes array */
      delete[] s;

      /* return the result */
      return average_silhouette;
    }


    /* method that resets a cluster back to its initialized state */
    void reset_clusters(void)
    {
      /* reset the radius and silhouettes */
      radius = 0.0;
      silhouette = 0.0;
      silhouettes->clear();

      /* reset all the cluster centers */
      for (int c = 0; c < K; c++)
      {
        centers[c]->reset_cluster_center();
      }
    }


    /* method that returns a pointer to a vector with all the images inside a specified cluster */
    std::vector<Item<T>*>* get_vectors_in_cluster(const uint16_t& cluster)
    {
      /* check for wrong input */
      if (cluster > K)
      {
        std::cout << "Error in calling Clustering::get_vectors_in_cluster(): Cluster " << cluster << " does not exist." << std::endl;
        return NULL;
      }

      /* if input is valid, return a pointer to the vectors in that specific cluster */
      return centers[cluster]->get_vectors_inside_cluster();
    }


    /* method used to retrieve the silhouette of each centroid */
    double* get_silhouettes(void)
    {
      /* create an array for the silhouettes */
      double* silhouettes = new double[K];

      /* get the silhouette of each cluster */
      for (int c = 0; c < K; c++)
      {
        silhouettes[c] = centers[c]->get_silhouette();
      }

      /* return the array */
      return silhouettes;
    }


    /* method that returns a pointer to a vector that contains the size of each cluster */
    std::vector<int> get_cluster_sizes(void)
    {
      /* create the vector that will be returned */
      std::vector<int> sizes;

      /* go through each center to build the vector with the sizes */
      for (int c = 0; c < K; c++)
      {
        sizes.push_back(centers[c]->get_cluster_size());
      }

      /* return the pointer to the vector */
      return sizes;
    }


    /* method that returns a pointer to a vector that contains the components of each centroid */
    std::vector<T*> get_centroids_components(void)
    {
      /* create the vector that will be returned */
      std::vector<T*> centroids;

      /* go through each center to build the vector with the sizes */
      for (int c = 0; c < K; c++)
      {
        centroids.push_back(centers[c]->get_components());
      }

      /* return the pointer to the vector */
      return centroids;
    }


    /* method that returns an array where each entry is an array of items inside a cluster */
    std::vector<Item<T>*>** get_items_per_cluster(void)
    {
      /* create an array of vectors of clusters */
      std::vector<Item<T>*>** items = new std::vector<Item<T>*>*[K];

      /* fill this array with the appropriate items */
      for (int c = 0; c < K; c++)
      {
        items[c] = get_vectors_in_cluster(c);
      }

      /* return the variable */
      return items;
    }


    /* method used to build the output object that will be "written" in the outfile */
    void build_output(interface::output::clustering::ClusteringOutput& output, const interface::Data<T>& data, interface::input::clustering::ClusteringInput& input, const double& clustering_time)
    {
      /* assign the correct values to the fields */
      output.K = K;
      output.d = data.dimension;
      output.method = input.algorithm;
      output.cluster_sizes = get_cluster_sizes();
      output.centroids = get_centroids_components();
      output.clustering_time = clustering_time;
      output.cluster_silhouettes = get_silhouettes();
      output.total_silhouette = silhouette;
      output.complete = input.complete;
      output.items = get_items_per_cluster();
    }


    /* method that frees the memory which the get_items_per_cluster() created */
    void free_output_object_memory(interface::output::clustering::ClusteringOutput& output)
    {
      /* delete the arrays */
      delete[] output.cluster_silhouettes;
      delete[] output.items;
    }
  };
}



#endif
