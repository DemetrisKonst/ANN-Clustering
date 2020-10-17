#include <unordered_map>

#include "../LSH/LSHFun.hpp"

class Hypercube {
private:
  int n;
  int HCdim;
  int datadim;
  double searchRadius;
  int m;

  std::vector<HashFunction> LSHFun;

  std::vector<uint8_t*>* H;
public:
  Hypercube(int n, int hcd, int dd, double sr, unsigned long int m, uint8_t** data):
  n(n), HCdim(hcd), datadim(dd), searchRadius(sr), m(m){
    int vertexCount = pow(2, HCdim);

    std::cout << vertexCount << '\n';

    H = new std::vector<uint8_t*>[vertexCount];

    for (int i = 0; i < vertexCount; i++) {
      std::vector<uint8_t*> tmpVec;
      H[i] = tmpVec;
    }

    for (int i = 0; i < HCdim; i++) {
      LSHFun.push_back(HashFunction(searchRadius, 4, 4, datadim, m));
    }

    for (int i = 0; i < n; i++) {
      int vertex = calculateVertex(data[i]);
      if (vertex >= vertexCount) std::cout << vertex << '\n';
      H[vertex].push_back(data[i]);

      if (i%10000 == 0)
        std::cout << "Calculated " << i << " records..." << '\n';
    }

    for (int i = 0; i < vertexCount; i++) {
      std::cout << i << "->" << H[i].size() << " records" << '\n';
    }
  }

  int calculateVertex (uint8_t* x) {
    int vertex = 0;
    for (int i = 0; i < HCdim; i++) {
      uint8_t hashRes = LSHFun[i].HashVector(x);

      // std::cout << "+=" << (int)hashRes << '\n';

      if (hashRes%2 == 0) {
        // vertex |= 0;
      }else{
        vertex |= 1;
      }

      if (i < HCdim-1)
        vertex <<= 1;

      // std::cout << vertex << '\n';
    }

    return vertex;
  }
};
