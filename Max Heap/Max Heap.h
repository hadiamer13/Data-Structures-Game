
#pragma once
#include <iostream>
using namespace std;

class MaxHeap
{
private:
    int *arr;
    int capacity;
    int size;

    void resize()
    {
        int newCapacity = capacity * 2;
        int *temp = new int[newCapacity];

        for (int i = 0; i < size; i++)
            temp[i] = arr[i];

        delete[] arr;

        arr = temp;
        capacity = newCapacity;
    }

    void swapValues(int &a, int &b)
    {
        int temp = a;
        a = b;
        b = temp;
    }

public:
    MaxHeap(int cap = 10)
    {
        capacity = cap;
        size = 0;
        arr = new int[capacity];
    }

    void insert(int value)
    {
        if (size == capacity)
            resize();

        int index = size;
        arr[index] = value;
        size++;

        while (index > 0)
        {
            int parent = (index - 1) / 2;

            if (arr[parent] >= arr[index])
                break;

            swapValues(arr[parent], arr[index]);
            index = parent;
        }
    }

    void heapify(int index)
    {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < size && arr[left] > arr[largest])
            largest = left;

        if (right < size && arr[right] > arr[largest])
            largest = right;

        if (largest != index)
        {
            swapValues(arr[index], arr[largest]);
            heapify(largest);
        }
    }

    int getMax()
    {
        if (size == 0)
            return -1;

        return arr[0];
    }

    int extractMax()
    {
        if (size == 0)
            return -1;

        int maximum = arr[0];

        arr[0] = arr[size - 1];
        size--;

        if (size > 0)
            heapify(0);

        return maximum;
    }

    void deleteValue(int value)
    {
        int index = -1;

        for (int i = 0; i < size; i++)
        {
            if (arr[i] == value)
            {
                index = i;
                break;
            }
        }

        if (index == -1)
            return;

        arr[index] = arr[size - 1];
        size--;

        if (index < size)
        {
            if (index > 0 && arr[index] > arr[(index - 1) / 2])
            {
                while (index > 0)
                {
                    int parent = (index - 1) / 2;

                    if (arr[parent] >= arr[index])
                        break;

                    swapValues(arr[parent], arr[index]);
                    index = parent;
                }
            }
            else
                heapify(index);
        }
    }

    void buildHeap(int values[], int n)
    {
        while (size < n)
            insert(values[size]);

        for (int i = size / 2 - 1; i >= 0; i--)
            heapify(i);
    }

    void display()
    {
        for (int i = 0; i < size; i++)
            cout << arr[i] << " ";

        cout << endl;
    }

    int getSize()
    {
        return size;
    }

    static void heapSort(int values[], int n)
    {
        for (int i = n / 2 - 1; i >= 0; i--)
            sortHeapify(values, n, i);

        for (int i = n - 1; i > 0; i--)
        {
            int temp = values[0];
            values[0] = values[i];
            values[i] = temp;

            sortHeapify(values, i, 0);
        }
    }

    static void sortHeapify(int values[], int n, int index)
    {
        int largest = index;
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < n && values[left] > values[largest])
            largest = left;

        if (right < n && values[right] > values[largest])
            largest = right;

        if (largest != index)
        {
            int temp = values[index];
            values[index] = values[largest];
            values[largest] = temp;

            sortHeapify(values, n, largest);
        }
    }

    ~MaxHeap()
    {
        delete[] arr;
    }
};
