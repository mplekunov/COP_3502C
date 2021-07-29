/* COP 3502C Programming Assignment3 
This program is written by: Mikhail Plekunov */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef struct Point
{
    int x;
    int y;
} Point;

Point *mainPos;

void insertionSort(Point *const *const arr, int low, int high);
void swap(Point *const point_1, Point *const point_2);

void mergeSort(Point *const *const arr, int low, int high, int threshold);
void merge(Point *const *const arr, int low, int middle, int high);

int search(Point *const *const arr, int low, int high, Point *const point);

int compareTo(Point *const point_1, Point *const point_2);

double calculateDistance(Point *const point_1, Point *const point_2);

Point **getData(FILE *fileptr, int size);
void free_pointer_arr(Point **arr, int size);

void sort(Point *const *const arr, int arr_size, int sort_threshold);

int main()
{
    FILE *input_file = fopen("assignment3input.txt", "r");

    mainPos = malloc(sizeof(*mainPos));
    int num_of_infected = 0;
    int sort_threshold = 0;
    int num_of_queries = 0;

    fscanf(input_file, "%d %d %d %d %d", &mainPos->x, &mainPos->y, &num_of_infected, &sort_threshold, &num_of_queries);

    Point **infected = getData(input_file, num_of_infected);
    Point **queries = getData(input_file, num_of_queries);

    sort(infected, num_of_infected, sort_threshold);

    FILE *output_file = fopen("out.txt", "w");

    for (size_t i = 0; i < num_of_infected; i++)
        fprintf(output_file, "%d %d\n", infected[i]->x, infected[i]->y);

    for (size_t i = 0; i < num_of_queries; i++)
    {
        int infected_index = search(infected, 0, num_of_infected - 1, queries[i]);
        
        if (infected_index == -1)
            fprintf(output_file, "%d %d not found\n", queries[i]->x, queries[i]->y);
        else
            fprintf(output_file, "%d %d found at rank %d\n", queries[i]->x, queries[i]->y, infected_index);
    }

    free_pointer_arr(infected, num_of_infected);
    free_pointer_arr(queries, num_of_queries);
    free(mainPos);

    fclose(input_file);
    fclose(output_file);

    return 0;
}

//Sort wrapper
//Sorts array in ascending order (closer points to the mainPoint come first)
void sort(Point *const *const arr, int arr_size, int sort_threshold)
{
    mergeSort(arr, 0, arr_size - 1, sort_threshold);
}

//Frees memory allocated by double pointer
void free_pointer_arr(Point **arr, int size)
{
    for (size_t i = 0; i < size; i++)
        free(arr[i]);

    free(arr);
}

//Reads data from file and assigns it to the double pointer
Point **getData(FILE *fileptr, int size)
{
    Point **collection = calloc(size, sizeof(*collection));

    for (size_t i = 0; i < size; i++)
    {
        collection[i] = malloc(sizeof(collection[i]));
        fscanf(fileptr, "%d %d", &collection[i]->x, &collection[i]->y);
    }

    return collection;
}

//Searches for a specific point inside of an array
int search(Point *const *const arr, int low, int high, Point *const point)
{
    if (low <= high)
    {
        int middle = (low + high) / 2;

        int comparisonReuslt = compareTo(arr[middle], point);

        if (comparisonReuslt == 0)
            return middle + 1;

        if (comparisonReuslt == 1)
            return search(arr, low, middle - 1, point);

        return search(arr, middle + 1, high, point);
    }

    return -1;
}

//Insertion Sorting Algorithm
void insertionSort(Point *const *const arr, int low, int high)
{
    for (int i = low + 1; i <= high; i++)
    {
        for (int j = i - 1; j >= 0; j--)
        {
            int comparisonResult = compareTo(arr[j + 1], arr[j]);

            if (comparisonResult == -1)
                swap(arr[j + 1], arr[j]);
        }
    }
}

//swaps values of two points
void swap(Point *const point_1, Point *const point_2)
{
    Point *temp_point = malloc(sizeof(*temp_point));

    temp_point->x = point_1->x;
    temp_point->y = point_1->y;

    point_1->x = point_2->x;
    point_1->y = point_2->y;

    point_2->x = temp_point->x;
    point_2->y = temp_point->y;

    free(temp_point);
}

//Merge Sorting Alogrithm
void mergeSort(Point *const *const arr, int low, int high, int sort_threshold)
{

    if (low < high)
    {
        int middle = (low + high) / 2;

        if (high - low + 1 <= sort_threshold)
            insertionSort(arr, low, high);
        else
        {
            mergeSort(arr, low, middle, sort_threshold);
            mergeSort(arr, middle + 1, high, sort_threshold);
        }

        merge(arr, low, middle, high);
    }
}

//Compares two points by calculation of distance betwenn each of them and the main point
//Returns -1 if first point is closer to the main point
//Returns 1 if second point is closer to the main point
//Returns 0 if both points are at the same distance (special case)
int compareTo(Point *const point_1, Point *const point_2)
{
    double distance_1 = calculateDistance(mainPos, point_1);
    double distance_2 = calculateDistance(mainPos, point_2);

    if (distance_1 < distance_2)
        return -1;
    else if (distance_1 == distance_2)
    {
        if (point_1->x < point_2->x)
            return -1;
        else if (point_1->x > point_2->x)
            return 1;
        else
        {
            if (point_1->y < point_2->y)
                return -1;
            else if (point_1->y > point_2->y)
                return 1;
            else
                return 0;
        }
    }
    else
        return 1;
}

//Calculates distance between two points
double calculateDistance(Point *const point_1, Point *const point_2)
{
    return sqrt(pow(point_1->x - point_2->x, 2) + pow(point_1->y - point_2->y, 2));
}

//Merge function for merge Sorting algorithm
void merge(Point *const *const arr, int low, int middle, int high)
{
    int i = low, j = middle + 1, k = 0;

    Point sorted[high - low + 1];

    while (i <= middle && j <= high)
    {
        int comparisonResult = compareTo(arr[i], arr[j]);

        if (comparisonResult == -1 || comparisonResult == 0)
        {
            sorted[k] = *arr[i];
            i++;
        }
        else
        {
            sorted[k] = *arr[j];
            j++;
        }

        k++;
    }

    while (i <= middle)
    {
        sorted[k] = *arr[i];
        k++;
        i++;
    }

    while (j <= high)
    {
        sorted[k] = *arr[j];
        k++;
        j++;
    }

    for (size_t i = low; i <= high; i++)
        *arr[i] = sorted[i - low];
}