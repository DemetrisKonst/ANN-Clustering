#pragma once

template <typename T>
bool comparePairs (std::pair<int, Item<T>*> x, std::pair<int, Item<T>*> y) {
  return (x.first < y.first);
}
