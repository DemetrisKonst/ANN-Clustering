#pragma once

template <typename T>
struct Item {
  // Unique identifier for each item
  long int id;
  // Data dimension
  int D;
  // Actual data
  T* data;
  // Flag for reverse assignment
  bool staged;
  // Flag for reverse assignment
  bool marked;
  // Flag if is NULL
  bool null;

  Item () {
    id = -1;
    D = -1;
    data = NULL;
    staged = false;
    marked = false;
    null = true;
  }

  Item (long int id, int d, T* data, bool st = false, bool md = false) : id(id), D(d), data(data), staged(st), marked(md) {
    null = false;
  }

  void operator = (const Item<T> &item) {
    id = item.id;
    D = item.D;
    data = item.data;
    staged = item.staged;
    marked = item.marked;
  }

  bool operator == (const Item<T> &item) {
    return id == item.id;
  }
};
