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

<<<<<<< HEAD
//Customer struct representing each customer
=======
>>>>>>> d4dd9a55cf33494190c293637da442ba670c3995
typedef struct Customer
{
    char *name;
    int num_of_items;
    int arrival_time;
    int line_number;
} Customer;

<<<<<<< HEAD
//Node struct
=======
>>>>>>> d4dd9a55cf33494190c293637da442ba670c3995
typedef struct Node
{
    Customer *customer;
    struct Node *next;
} Node;

<<<<<<< HEAD
//Queue struct representing each line in store
=======
>>>>>>> d4dd9a55cf33494190c293637da442ba670c3995
typedef struct Queue
{
    Node *front, *back;
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
Customer *peek(Queue *queue);
int isEmpty(Queue *queue);

Node *newNode(Customer *customer);
<<<<<<< HEAD

=======
>>>>>>> d4dd9a55cf33494190c293637da442ba670c3995
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
    file->current_line++;

    testLimits(num_cases, MAX_NUM_OF_TEST_CASES, false, file, "Case Number");

    //iterates through all test cases
    //Sorts customers by their checkout line
    //Checkout customers
    for (size_t i = 0; i < num_cases; i++)
    {
        int num_customers;
        fscanf(file->fptr, "%d", &num_customers);
        file->current_line++;

        testLimits(num_customers, MAX_NUM_OF_CUSTOMERS, false, file, "Number of Customers");

        Queue *lines = sortCustomers(file, num_customers);

        checkoutCustomers(lines, num_customers);

        free(lines);
    }

    closeFile(file);

    return 0;
}

/*
    Tests if the queue is empty
    Returns test result
*/
int isEmpty(Queue *queue)
{
    if (queue->front == NULL)
        return 1;

    return 0;
}

/*
    look at the first (front) node in the queue
    Returns that node 
*/
Customer *peek(Queue *queue)
{
    if (isEmpty(queue))
        return NULL;
    else
        return queue->front->customer;
}

/*
    Adds customer into the queue node
*/
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

/*
    Removes node from the queue
    Address of the customer remains untouched
*/
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

/*
    Creates new node and assigns customer to it
    Returns an address of the node
*/
Node *newNode(Customer *customer)
{
    Node *temp = malloc(sizeof(*temp));
    temp->customer = customer;
    temp->next = NULL;
    return temp;
}

/*
    Calculates checkout time
*/
int calculateCheckoutTime(Customer *customer, int current_time)
{
    return current_time + customer->num_of_items * BASE_TIME_TO_CHECK_ONE_ITEM + BASE_TIME_TO_CHECK_CUSTOMER;
}

/*
    Simulate checkout order for all customers in all queues (lines)
*/
void checkoutCustomers(Queue *queues, int num_customers)
{
    int current_time = 0;

    //As longs as there are customers, iterates through each of them
    for (size_t i = 0; i < num_customers; i++)
    {
        //stores the numbers of lines that have customers in it
        int *occupied_store_lines = NULL;
        //stores the max index of occupied_store_lines array
        size_t max_index = 0;

        //current customer to be served
        Customer *current_customer = NULL;

        //Looks for lines that have customers who arrived before current_time
        //Adds line nunbers of those customers to occupied_store_lines array
        for (size_t j = 0; j < MAX_NUMBER_OF_LINES; j++)
        {
            if (!isEmpty(&queues[j]))
            {
                Customer *customer = peek(&queues[j]);

                if (customer->arrival_time < current_time)
                {
                    occupied_store_lines = realloc(occupied_store_lines, sizeof(*occupied_store_lines) * (max_index + 1));
                    occupied_store_lines[max_index++] = customer->line_number - 1;
                }
            }
        }

        //If there are currently customers who arrived before current time, figures out who to serve first
        //If there are currently no customers, "awaits" for the first customer to come 
        if (occupied_store_lines != NULL)
        {
            //Iterates as many times, as there are lines in an array
            for (size_t j = 0; j < max_index; j++)
            {
                Customer *customer = peek(&queues[occupied_store_lines[j]]);

                if (current_customer == NULL || current_customer->num_of_items > customer->num_of_items ||
                    (current_customer->num_of_items == customer->num_of_items && current_customer->line_number > customer->line_number))
                {    
                    current_customer = customer;
                }
            }
        }
        else
        {
            //Iterates through all lines in the store to "wait"/"look" for new customer
            for (size_t j = 0; j < MAX_NUMBER_OF_LINES; j++)
            {
                if (!isEmpty(&queues[j]))
                {
                    Customer *customer = peek(&queues[j]);

                    if (current_customer == NULL || current_customer->arrival_time > customer->arrival_time)
                        current_customer = customer;
                }
            }
        }

        deQueue(&queues[current_customer->line_number - 1]);

        //If there are no customers at current_time, then it waits untill fist customer appears
        //Therefore it will wait untill current_time == arrival_time of the first customer to appear
        if (occupied_store_lines == NULL)
            current_time = current_customer->arrival_time;

        current_time = calculateCheckoutTime(current_customer, current_time);

        printf("%s from line %d checks out at time %d.\n",
               current_customer->name, current_customer->line_number, current_time);

        free(current_customer->name);
        free(current_customer);
        free(occupied_store_lines);
    }
}

/*
    Sort customers by lines
    Returns filled collection of lines
*/
Queue *sortCustomers(FileInfo *file, int num_customers)
{
    Queue *lines = malloc(sizeof(*lines) * MAX_NUMBER_OF_LINES);

    //Initializes all lines to NULL
    for (size_t i = 0; i < MAX_NUMBER_OF_LINES; i++)
        lines[i].back = lines[i].front = NULL;

    //for each customer in the file takes customer information
    //Tests it for limits
    //puts it into the queue corresponding to the line number 
    for (size_t i = 0; i < num_customers; i++)
    {
        int arrival_time, line, num_of_items;
        char *name = malloc(sizeof(*name) * MAX_NAME_LENGTH);

        fscanf(file->fptr, "%d %d %s %d", &arrival_time, &line, name, &num_of_items);
        file->current_line++;

        testLimits(arrival_time, MAX_TIME_IN_SECONDS, true, file, "Arrival Time");
        testLimits(line, MAX_NUMBER_OF_LINES, false, file, "Line Number");
        testLimits(strlen(name), MAX_NAME_LENGTH, false, file, "Name Length");
        testLimits(num_of_items, MAX_NUMBER_OF_ITEMS, false, file, "Number of Items");

        name = realloc(name, strlen(name) + 1);

        Customer *customer = malloc(sizeof(*customer));

        customer->name = strdup(name);
        customer->arrival_time = arrival_time;
        customer->line_number = line;
        customer->num_of_items = num_of_items;

        enQueue(&lines[--line], customer);

        free(name);
    }

    return lines;
}

/*
    Closes file and frees memory
*/
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
    file->current_line = 0;

    return file;
}