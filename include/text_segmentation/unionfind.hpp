#pragma once

class UnionFind
{
    int *id, cnt, *sz;

    public:
        // Create an empty union find data structure with N isolated sets.
        UnionFind(int N);
        ~UnionFind();

        // Return the id of component corresponding to object p.
        int find(int p);

        // Replace sets containing x and y with their union.
        void merge(int x, int y);

        // Are objects x and y in the same set?
        bool connected(int x, int y);

        // Return the number of disjoint sets.
        int count();
};
