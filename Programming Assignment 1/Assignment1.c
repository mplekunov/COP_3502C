#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

//Student Struct
typedef struct Student
{
    int id;
    char *lname;
    float *scores;
    float std_avg;
} Student;

//Course Struct
typedef struct Course
{
    char *course_name;
    int num_sections;
    Student **sections;
    int *num_students;
    int *num_scores;
} Course;

/*
    Constants that define constraints of Values used in the code
*/
#define MAX_NUM_OF_TEST_CASES 25
#define MAX_NUM_OF_COURSES 500
#define MAX_COURSE_NAME_LENGTH 20
#define MAX_NUM_OF_SECTIONS 10
#define MAX_NUM_OF_STUDENTS_IN_SECTION 500
#define MAX_NUM_OF_ASSIGNMENTS_IN_SECTION 20
#define MAX_ID 50000
#define MAX_STUDENT_LASTNAME_LENGTH 20
#define MAX_SCORE 100
#define PASS_SCORE 70
#define INPUT_FILENAME "assignment1input.txt" 

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

//Enum defining data types
typedef enum
{
    Integer,
    Float
} DataType;

//Struct containing output information
typedef struct OutputInfo
{
    int pass_count;
    float *avg_scores_per_section;
    Student *student;
} OutputInfo;

Course *readCourses(FileInfo *file, int *num_courses);
Student **readSections(FileInfo *file, int num_students[], int num_scores[], int num_sections);
void process_courses(Course *courses, int num_courses);
void release_courses(Course *courses, int num_courses);

char *getWord(FileInfo *file);
void testLimits(double num_to_test, double limit, bool isZeroAllowed, FileInfo *file, const char *variable_name);
void errorHandler(FileInfo *file, InputFormatResult error, const char *variable_name);
FileInfo *openFile(const char *filename, const char *mode);
void closeFile(FileInfo *file);
void *StringToNumber(char *string, DataType data_type);
OutputInfo *countPassedStudents(Course *course);
void studentCpy(Student *dist, const Student *src, const int *num_scores);
OutputInfo *initOutputInfo(const Course *course);

int main()
{
    FileInfo *file = openFile(INPUT_FILENAME, "r");

    int *num_cases = (int *)StringToNumber(getWord(file), Integer);
    testLimits(*num_cases, MAX_NUM_OF_TEST_CASES, false, file, "Case Number");
    
    //iterates through all test cases
    for (size_t i = 0; i < *num_cases; i++)
    {
        int *num_courses = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_courses, MAX_NUM_OF_COURSES, false, file, "Course Number"); 

        //initialize courses dynamically allocated array with information about courses
        Course *courses = readCourses(file, num_courses);

        printf("\nTest Case %d\n", ((int)i + 1));
        process_courses(courses, *num_courses);

        release_courses(courses, *num_courses);

        free(num_courses);
    }

    closeFile(file);
    free(num_cases);

    return 0;
}

/*
    Frees memory of a dynamically allocated array courses represented as a pointer
    Takes Course pointer and number of courses stored in the array
*/
void release_courses(Course *courses, int num_courses)
{
    //iterates through courses
    for (size_t i = 0; i < num_courses; i++)
    {
        Course *current_course = &courses[i];
        
        //iterates through each section withing course
        for (size_t j = 0; j < current_course->num_sections; ++j)
        {
            //iterates through each student in the section
            //frees pointers
            for (size_t k = 0; k < current_course->num_students[j]; k++)
            {
                free(current_course->sections[j][k].lname);
                free(current_course->sections[j][k].scores);
            }

            free(current_course->sections[j]);            
        }

        free(current_course->sections);
        free(current_course->course_name);
        free(current_course->num_scores);
        free(current_course->num_students);
    }

    free(courses);
}

/*
    Outputs information about students from courses dynamically allocated array using OuputInfo structure
    Takes Course pointer and number of courses stored in the array
*/
void process_courses(Course *courses, int num_courses)
{
    //iterates through all courses
    for (size_t i = 0; i < num_courses; i++)
    {
        Course *current_course = &courses[i];
        OutputInfo *out_info = countPassedStudents(current_course);

        printf("%s %d", current_course->course_name, out_info->pass_count);

        //iterates through each section within course
        for (size_t j = 0; j < current_course->num_sections; j++)
            printf(" %.2f ", out_info->avg_scores_per_section[j]);

        printf("%d %s %.2f\n", out_info->student->id, out_info->student->lname, out_info->student->std_avg);

        free(out_info->student->lname);
        free(out_info->student->scores);
        free(out_info->student);

        free(out_info->avg_scores_per_section);
        free(out_info);
    }
}

/*
    Initialize memory for OutputInfo Structure
    Takes Course pointer
    Returns pointer to OutputInfo structure
*/
OutputInfo *initOutputInfo(const Course *course)
{
    //By default initializes student with information about first student in the first section of the course
    Student *student = &(course->sections[0][0]);
    int *num_scores = &(course->num_scores[0]);

    OutputInfo *out_info = malloc(sizeof(*out_info));

    out_info->pass_count = 0;

    out_info->avg_scores_per_section = malloc(course->num_sections * sizeof(*out_info->avg_scores_per_section));
    
    out_info->student = malloc(sizeof(*out_info->student));
            
    out_info->student->id = student->id;
    
    out_info->student->lname = malloc((strlen(student->lname) + 1) * sizeof(*student->lname));
    
    strcpy(out_info->student->lname, student->lname);

    out_info->student->scores = malloc(*num_scores * sizeof(*out_info->student->scores));

    memcpy(out_info->student->scores, student->scores, course->num_scores[0] * sizeof(*student->scores));
    
    out_info->student->std_avg = student->std_avg;

    return out_info;
}

/*
    Counts number of students that passed course
    Takes Course pointer
    Returns OutputInfo
*/
OutputInfo *countPassedStudents(Course *course)
{
    OutputInfo *out_info = initOutputInfo(course);
    
    //iterates through each section within course
    for (size_t i = 0; i < course->num_sections; i++)
    {
        float *avg_scores = &(out_info->avg_scores_per_section[i]);
        *avg_scores = 0;

        //iterates through each student within couse
        //compares student's average grade to the out-info's student
        //if it's higher, copies new student info to the out-info's student
        for (size_t j = 0; j < course->num_students[i]; j++)
        {
            Student *student = &(course->sections[i][j]);

            if (student->std_avg >= PASS_SCORE)
            {
                out_info->pass_count++;

                if (out_info->student->std_avg < student->std_avg)
                    studentCpy(out_info->student, student, &(course->num_scores[i]));
            }

            (*avg_scores) += student->std_avg;
        }

        (*avg_scores) /= *(course->num_students + i);
    }

    return out_info;
}

/*
    Copies one Student structure into another
    Take Student pointer dist -> distination, constant Student pointer src -> source and number of scores in the src Student structure
*/
void studentCpy(Student *dist, const Student *src, const int *num_scores)
{
    dist->id = src->id;

    dist->lname = realloc(dist->lname, (strlen(src->lname) + 1) * sizeof(*dist->lname));

    strcpy(dist->lname, src->lname);

    dist->std_avg = src->std_avg;

    dist->scores = realloc(dist->scores, *num_scores * sizeof(*src->scores));

    memcpy(dist->scores, src->scores, *num_scores * sizeof(*src->scores));
}

/*
    Reads data from the file, tests data for constraints, allocates memory for the double pointer which holds data read from the file
    Takes FileInfo pointer, array representing number of students per course, array representing number of scores per course, number of sections
    Returns double pointer which represents a 2D array containing sections and students
*/
Student **readSections(FileInfo *file, int students[], int scores[], int num_sections)
{
    Student **sections_and_students = (Student **)malloc(num_sections * sizeof(Student *));

    //iterates through sections
    for (size_t i = 0; i < num_sections; i++)
    {
        int *num_students = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_students, MAX_NUM_OF_STUDENTS_IN_SECTION, false, file, "Student Number");

        int *num_scores = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_scores, MAX_NUM_OF_ASSIGNMENTS_IN_SECTION, false, file, "Assignment Number");

        students[i] = *num_students;
        scores[i] = *num_scores;

        sections_and_students[i] = malloc(*num_students * sizeof(*sections_and_students[i]));

        //iterates throguh each stdent within section
        //reads information about that student from the file
        //assigns it to the sections_and_student pointer
        for (size_t j = 0; j < *num_students; j++)
        {
            Student *student = &sections_and_students[i][j];

            int *id = (int *)StringToNumber(getWord(file), Integer);
            testLimits(*id, MAX_ID, true, file, "Student ID");
            
            student->id = *id;
            free(id);

            student->lname = getWord(file);
            testLimits((int)strlen(student->lname), MAX_STUDENT_LASTNAME_LENGTH, false, file, "Student LastName");

            float score_avg = 0;

            student->scores = malloc(*num_scores * sizeof(*student->scores));

            for (size_t k = 0; k < *num_scores; k++)
            {
                float *score = (float *)StringToNumber(getWord(file), Float);
                testLimits(*score, MAX_SCORE, true, file, "Student Score");

                score_avg += *score;

                student->scores[k] = *score;

                free(score);
            }

            student->std_avg = score_avg / *num_scores;
        }

        free(num_students);
        free(num_scores);
    }

    return sections_and_students;
}

/*
    Reads courses from the file
    Takes FileInfo pointer and number of courses
    Returns Course pointer representing a dynamically allocated array of courses
*/
Course *readCourses(FileInfo *file, int *num_courses)
{
    Course *courses = malloc(sizeof(*courses) * (*num_courses));

    //iterates through eacg course in the file
    //puts information about each course into the courses pointer 
    for (size_t i = 0; i < *num_courses; i++)
    {
        Course *current_course = &courses[i];

        current_course->course_name = getWord(file);
        testLimits((int)strlen(current_course->course_name), MAX_COURSE_NAME_LENGTH, false, file, "Course Name");

        int *num_sections = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_sections, MAX_NUM_OF_SECTIONS, false, file, "Section Number");
        
        current_course->num_sections = *num_sections;
        free(num_sections);

        current_course->num_students = malloc(current_course->num_sections * sizeof(*current_course->num_students));
        current_course->num_scores = malloc(current_course->num_sections * sizeof(*current_course->num_scores));

        current_course->sections = readSections(file, current_course->num_students, current_course->num_scores, current_course->num_sections);
    }

    return courses;
}

/*
    Takes string containing number and desired data type
    Returns a desired data type pointer
*/
void *StringToNumber(char *string, DataType data_type)
{
    if (data_type == Integer)
    {
        int *num = malloc(sizeof(*num));
        *num = atoi(string);
        free(string);
        return num;
    }
    else if (data_type == Float)
    {
        float *num = malloc(sizeof(*num));
        *num = atof(string);
        free(string);
        return num;
    }
    else
    {
        printf("Passed wrong data type to the StringToNumber function...");
        exit(EXIT_FAILURE);
    }
}

/*
    Closes file, fres FileInfo structure
*/
void closeFile(FileInfo *file)
{
    fclose(file->fptr);
    free(file);
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
    Reads line from input buffer
    Returns pointer to the char containing that line
*/
char *getWord(FileInfo *file)
{
    char *lineptr = NULL;
    size_t size = 0, index = 0;
    int ch = EOF;

    while (ch)
    {
        ch = getc(file->fptr);

        if ((index == 0 && ch == ' ') || (index == 0 && ch == '\n'))
            continue;
        else if (ch == EOF || ch == '\n' || ch == ' ')
        {
            if (ch == '\n')
                file->current_line++;

            ch = '\0';
        }

        if (size <= index)
        {
            size += sizeof(char);

            lineptr = realloc(lineptr, size);
        }

        lineptr[index++] = ch;
    }

    return lineptr;
}