#pragma once

#include <utility>

#include "../metrics/metrics.hpp"
#include "../LSH/LSHFun.hpp"

namespace utils {
  /*
  Helper compare function.
  Used in kNN of brute force, LSH and Hypercube
  */
  template <typename T>
  bool comparePairs (std::pair<int, Item<T>*> x, std::pair<int, Item<T>*> y) {
    return (x.first < y.first);
  }

  /*
  Method to calculate the averageDistance between a portion of the items.
  This is used to calculate a "proper" window size for LSH & Hypercube
  */
  template <typename T>
  double averageDistance (double datasetPercentage, Item<T>** items, int totalImageCount, int dimension) {
    // Calculate the size of the partition
    int partitionSize = floor(datasetPercentage*totalImageCount);

    // For each item inside the partition...
    double outerSum = 0.0;
    for (int i = 0; i < partitionSize; i++) {
      int innerSum = 0;
      // Traverse each other item inside the partition
      for (int j = 0; j < partitionSize; j++) {
        if (i == j) continue;
        // Calculate the distance between the two items (and summarize it)
        innerSum += metrics::ManhattanDistance<T>(items[i]->data, items[j]->data, dimension);
      }
      // Then calculate the average distance of 1 item from all other items
      outerSum += 1.0*innerSum/partitionSize;
    }

    // Then calculate the average distance of all items from all other items
    return outerSum/partitionSize;
  }

  /*
  Method which returns a pair of mean, deviation for a particular dataset.
  The user can provide the (proportional) size of the dataset to calculate its mean and deviation.
  Used in Hypercube to calculate F_Range.
  */
  template <typename T>
  std::pair<double, double> calculateMeanDeviation (double datasetPercentage, Item<T>** items, int totalImageCount, int dimension, int hashAmount, int windowSize) {
    // Calculate the size of the partition
    int partitionSize = floor(datasetPercentage*totalImageCount);

    // Initialize a table of LSH HashFunctions
    HashFunction<T>** hFun = new HashFunction<T>*[hashAmount];

    // Initialize m_mod_M values
    unsigned int mConstant = pow(2, 32) - 5;
    int* m_mod_MValues = new int[dimension];
    for (int b = 0; b < dimension; b++)
      m_mod_MValues[b] = utils::modEx(mConstant, dimension-b-1, pow(2, 32/hashAmount));

    // Initialize all HashFunctions with the parameters given
    for (int j = 0; j < hashAmount; j++) {
      hFun[j] = new HashFunction<T>(windowSize, hashAmount, dimension, m_mod_MValues);
    }

    // First calculate the mean among all HashFunction values
    double meanSum = 0;
    for (int i = 0; i < partitionSize; i++) {
      /*
      Since we have more than 1 values for every item, we need to calculate an average
      of these HashFunction values. That average will be added to the outer sum which helps
      calculate the actual mean.
      */
      int innerSum = 0;
      for (int j = 0; j < hashAmount; j++) {
        innerSum += hFun[j]->HashVector(items[i]->data);
      }
      double innerAverage = innerSum/hashAmount;

      meanSum += innerAverage;
    }
    double mean = meanSum / partitionSize;

    // Second calculate the variance among all HashFunction values
    double varianceSum = 0;
    for (int i = 0; i < partitionSize; i++) {
      /*
      As implemented above, since we have more than 1 values for every item, we need to
      calculate an average of these HashFunction values.
      That average will be added to the outer sum which helps
      calculate the actual variance.
      */
      int innerSum = 0;
      for (int j = 0; j < hashAmount; j++) {
        innerSum += hFun[j]->HashVector(items[i]->data);
      }
      double innerAverage = innerSum/hashAmount;

      varianceSum += pow(innerAverage - mean, 2);
    }
    double variance = varianceSum / partitionSize;

    double deviation = sqrt(variance);
    return std::make_pair(mean, deviation);
  }
}
