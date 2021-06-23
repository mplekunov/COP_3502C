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
Node *newNode(Customer *customer);
Node *peek(Queue *queue);
int isEmpty(Queue *queue);
Queue *createQueue();

FileInfo *openFile(const char *filename, const char *mode);
void closeFile(FileInfo *file_info);
void testLimits(double num_to_test, double limit, bool isZeroAllowed, FileInfo *file, const char *variable_name);
void errorHandler(FileInfo *file, InputFormatResult error, const char *variable_name);

Queue *sortCustomers(FileInfo *file, int num_customers);
Node *processCustomers(Queue *collection, int num_customers);
int calculateCheckoutTime(Customer *customer, int current_time);
void copyCustomer(Customer *dist, Customer *src);


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

        Node *checkoutOrder = processCustomers(lines, num_customers);

        while (checkoutOrder != NULL)
        {
            Node *temp = checkoutOrder;
            printf("Name: %s, Line: %d, Departure Time: %d\n", checkoutOrder->customer->name, checkoutOrder->customer->line_number, checkoutOrder->customer->departure_time);
            checkoutOrder = checkoutOrder->next;

            free(temp->customer->name);
            free(temp->customer);
            free(temp);   
        }

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
    return queue->front;
}

Queue *createQueue()
{
    Queue *queue = malloc(sizeof(*queue));
    queue->front = queue->back = NULL;
    return queue;
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

    free(front->customer->name);
    free(front->customer);
    free(front);
}

Node *newNode(Customer *customer)
{
    Node *temp = malloc(sizeof(*temp));
    temp->customer = malloc(sizeof(*temp->customer));
    copyCustomer(temp->customer, customer);
    temp->next = NULL;
    return temp;
}

int calculateCheckoutTime(Customer *customer, int current_time)
{
    return current_time + customer->num_of_items * BASE_TIME_TO_CHECK_ONE_ITEM + BASE_TIME_TO_CHECK_CUSTOMER;
}

void copyCustomer(Customer *dist, Customer *src)
{
    if (src != NULL)
    {
        dist->name = strdup(src->name);
        dist->arrival_time = src->arrival_time;
        dist->departure_time = src->departure_time;
        dist->line_number = src->line_number;
        dist->num_of_items = src->num_of_items;
    }
}

Node *processCustomers(Queue *lines, int num_customers)
{
    Customer *first_customer = NULL;
    Node *checkoutOrder = newNode(first_customer);
    
    //Customer *checkoutOrder = malloc(sizeof(*checkoutOrder) * num_customers);

    //Finds first customer
    for (size_t i = 0; i < MAX_NUMBER_OF_LINES; i++)
    {
        Node *current_node = peek(&lines[i]);
        if (current_node != NULL)
        {
            if (first_customer == NULL || first_customer->arrival_time > current_node->customer->arrival_time)
                first_customer = current_node->customer;
        }
    }

    first_customer->departure_time = calculateCheckoutTime(first_customer, first_customer->arrival_time);
    int arrival_time = first_customer->departure_time;

    copyCustomer(checkoutOrder->customer, first_customer);

    deQueue(&lines[first_customer->line_number - 1]);

    for (size_t i = 1; i < num_customers; i++)
    {
        Node *node = NULL;

        //Finds the customer with lowest number of items
        for (size_t j = 0; j < MAX_NUMBER_OF_LINES; j++)
        {
            Node *current_node = peek(&lines[j]);

            if (current_node != NULL)
            {
                if (node == NULL || node->customer->num_of_items > current_node->customer->num_of_items)
                    node = current_node;
            }
        }

        node->customer->departure_time = calculateCheckoutTime(node->customer, arrival_time);

        Node *last = checkoutOrder;
        while (last->next != NULL)
            last = last->next;

        last->next = newNode(node->customer);

        arrival_time = node->customer->departure_time;

        deQueue(&lines[node->customer->line_number - 1]);

    }

    return checkoutOrder;
}

Queue *sortCustomers(FileInfo *file, int num_customers)
{
    Queue *lines = malloc(sizeof(*lines) * MAX_NUMBER_OF_LINES);

    for (size_t i = 0; i < MAX_NUMBER_OF_LINES; i++)
    {
        lines[i].back = NULL;
        lines[i].front = NULL;
    }

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

        free(customer->name);
        free(customer);
        free(name);
    }

    return lines;
}

void freeCustomer(Customer *customer)
{
    free(customer->name);
    free(customer);
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