#include "../include/BruteForce/BruteForce.hpp"
#include "../include/LSH/LSH.hpp"
#include "../include/Hypercube/Hypercube.hpp"

template <typename T>
BruteForce<T>* initializeBF (interface::Data<T>& data) {
  return new BruteForce<T>(data);
}

template <typename T>
LSH<T>* intializeLSH (interface::input::LSH::LSHInput& input, interface::Data<T>& data) {
  return new LSH<T>(input, data);
}

template <typename T>
Hypercube<T>* initializeHC (interface::input::HC::HCInput& input, interface::Data<T>& data) {
  return new Hypercube<T>(input, data);
}
