/* COP 3502C Assignment 2
This program is written by: Mikhail Plekunov */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define BASE_TIME_TO_CHECK_CUSTOMER 30
#define BASE_TIME_TO_CHECK_ONE_ITEM 5
#define MAX_NUM_OF_TEST_CASES 25
#define MAX_NUM_OF_CUSTOMERS 500000
#define MAX_TIME_IN_SECONDS pow(10, 9)
#define MAX_NUMBER_OF_LINES 12
#define MAX_NAME_LENGTH 9
#define MAX_NUMBER_OF_ITEMS 100
#define INPUT_FILENAME "assignment2input.txt"

typedef struct Customer
{
    char *name;
    int num_of_items;
    int arrival_time;
    int departure_time;
    int line_number;
} Customer;

typedef struct Node
{
    Customer *customer;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node *front, *back; // 1 2 3
} Queue;

//Enum defining an out-of-range error used in constraints checking
typedef enum
{
    OUT_OF_RANGE
} InputFormatResult;

/*
    Information about the file containing pointer to the file
    And current line at which reading of the file stoped
*/
typedef struct FileInfo
{
    FILE *fptr;
    int current_line;
} FileInfo;

void enQueue(Queue *queue, Customer *customer);
void deQueue(Queue *queue);
Node *peek(Queue *queue);
int isEmpty(Queue *queue);

Node *newNode(Customer *customer);
FileInfo *openFile(const char *filename, const char *mode);
void closeFile(FileInfo *file_info);
void testLimits(double num_to_test, double limit, bool isZeroAllowed, FileInfo *file, const char *variable_name);
void errorHandler(FileInfo *file, InputFormatResult error, const char *variable_name);

Queue *sortCustomers(FileInfo *file, int num_customers);
void checkoutCustomers(Queue *queues, int num_customers);
int calculateCheckoutTime(Customer *customer, int current_time);

int main()
{
    FileInfo *file = openFile(INPUT_FILENAME, "r");

    int num_cases;
    fscanf(file->fptr, "%d", &num_cases);
    testLimits(num_cases, MAX_NUM_OF_TEST_CASES, false, file, "Case Number");

    //iterates through all test cases
    for (size_t i = 0; i < num_cases; i++)
    {
        int num_customers;
        fscanf(file->fptr, "%d", &num_customers);
        testLimits(num_customers, MAX_NUM_OF_CUSTOMERS, false, file, "Number of Customers");

        Queue *lines = sortCustomers(file, num_customers);

        checkoutCustomers(lines, num_customers);

        free(lines);
    }

    closeFile(file);

    return 0;
}

int isEmpty(Queue *queue)
{
    if (queue->front == NULL)
        return 1;

    return 0;
}

Node *peek(Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    else
        return queue->front;
}

void enQueue(Queue *queue, Customer *customer)
{
    Node *node = newNode(customer);

    if (queue->back == NULL)
        queue->back = queue->front = node;
    else
    {
        queue->back->next = node;
        queue->back = node;
    }
}

void deQueue(Queue *queue)
{
    if (queue->front == NULL)
        return;

    Node *front = queue->front;

    queue->front = queue->front->next;

    if (queue->front == NULL)
        queue->back = NULL;

    free(front);
}

Node *newNode(Customer *customer)
{
    Node *temp = malloc(sizeof(*temp));
    temp->customer = customer;
    temp->next = NULL;
    return temp;
}

int calculateCheckoutTime(Customer *customer, int current_time)
{
    return current_time + customer->num_of_items * BASE_TIME_TO_CHECK_ONE_ITEM + BASE_TIME_TO_CHECK_CUSTOMER;
}

void checkoutCustomers(Queue *queues, int num_customers)
{
    int current_time = 0;

    for (size_t i = 0; i < num_customers; i++)
    {
        int available_customers = 0;

        Customer *customer = NULL;

        for (size_t j = 0; j < MAX_NUMBER_OF_LINES; j++)
        {
            if (!isEmpty(&queues[j]))
            {
                Customer *current_customer = peek(&queues[j])->customer;

                if (current_customer->arrival_time < current_time)
                    available_customers++;
            }
        }

        int isFirst = 0;
        if (available_customers == 0)
            isFirst = 1;

        //Find customer
        for (size_t j = 0; j < MAX_NUMBER_OF_LINES; j++)
        {
            if (!isEmpty(&queues[j]))
            {
                Customer *current_customer = peek(&queues[j])->customer;
                //First customer

                if (available_customers > 0)
                {
                    if (current_customer->arrival_time < current_time)
                    {
                        if (customer == NULL || customer->num_of_items > current_customer->num_of_items ||
                            (customer->num_of_items == current_customer->num_of_items && customer->line_number > current_customer->line_number))
                            customer = current_customer;

                        if (--available_customers == 0)
                            break;
                    }
                }
                else
                {
                    if (customer == NULL || customer->arrival_time > current_customer->arrival_time)
                        customer = current_customer;
                }
            }
        }

        deQueue(&queues[customer->line_number - 1]);

        if (isFirst)
            current_time = customer->arrival_time;

        customer->departure_time = calculateCheckoutTime(customer, current_time);
        current_time = customer->departure_time;

        printf("Name: %s, Line: %d, Departure Time: %d\n", 
            customer->name, customer->line_number, customer->departure_time);

        free(customer->name);
        free(customer);
    }
}

Queue *sortCustomers(FileInfo *file, int num_customers)
{
    Queue *lines = malloc(sizeof(*lines) * MAX_NUMBER_OF_LINES);

    for (size_t i = 0; i < MAX_NUMBER_OF_LINES; i++)
        lines[i].back = lines[i].front = NULL;

    for (size_t i = 0; i < num_customers; i++)
    {
        int arrival_time, line, num_of_items;
        char *name = malloc(sizeof(*name) * MAX_NAME_LENGTH);

        fscanf(file->fptr, "%d %d %s %d", &arrival_time, &line, name, &num_of_items);

        testLimits(arrival_time, MAX_TIME_IN_SECONDS, true, file, "Arrival Time");
        testLimits(line, MAX_NUMBER_OF_LINES, false, file, "Line Number");
        testLimits(strlen(name), MAX_NAME_LENGTH, false, file, "Name Length");
        testLimits(num_of_items, MAX_NUMBER_OF_ITEMS, false, file, "Number of Items");

        name = realloc(name, strlen(name) + 1);

        Customer *customer = malloc(sizeof(*customer));

        customer->name = strdup(name);
        customer->arrival_time = arrival_time;
        customer->departure_time = -1;
        customer->line_number = line;
        customer->num_of_items = num_of_items;

        enQueue(&lines[--line], customer);

        free(name);
    }

    return lines;
}

void closeFile(FileInfo *file_info)
{
    fclose(file_info->fptr);
    free(file_info);
}

/*
    Test Number for a specific limit
    Takes number to test, limit to test this number against, boolean isZeroAllowed, FileInfo pointer and name of the variable in the code that is to be tested
    Throws an error if the number provided is out_of_range
    Error shows the line in the file where error was encountered as well as the name of the variable that caused an error
*/
void testLimits(double num_to_test, double limit, bool isZeroAllowed, FileInfo *file, const char *variable_name)
{
    if (isZeroAllowed)
    {
        if (num_to_test < 0 || num_to_test > limit)
            errorHandler(file, OUT_OF_RANGE, variable_name);
    }
    else
    {
        if (num_to_test <= 0 || num_to_test > limit)
            errorHandler(file, OUT_OF_RANGE, variable_name);
    }
}

/*
    Responsible for handling an error
    Terminates program on Error
*/
void errorHandler(FileInfo *file, InputFormatResult error, const char *variable_name)
{
    switch (error)
    {
    case OUT_OF_RANGE:
        printf("Error encountered on the line %d. %s is outside of allowed range!", file->current_line, variable_name);
        break;

    default:
        break;
    }

    closeFile(file);
    exit(EXIT_FAILURE);
}

/*
    Opens Files
    Takes filename and openning mode
    Returns FileInfo pointer representing information about file
*/
FileInfo *openFile(const char *filename, const char *mode)
{
    FileInfo *file = malloc(sizeof(*file));

    FILE *fptr = fopen(filename, mode);

    file->fptr = fptr;
    file->current_line = 1;

    return file;
}