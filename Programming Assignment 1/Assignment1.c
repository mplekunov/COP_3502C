#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Student
{
    int id;
    char *lname;
    float *scores;
    float std_avg;
} Student;

typedef struct Course
{
    char *course_name;
    int num_sections;
    Student **sections;
    int *num_students;
    int *num_scores;
} Course;

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

typedef enum
{
    OUT_OF_RANGE
} InputFormatResult;

typedef struct FileInfo
{
    FILE *fptr;
    int current_line;
} FileInfo;

typedef enum
{
    Integer,
    Float
} DataType;

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

    for (size_t i = 0; i < *num_cases; i++)
    {
        int *num_courses = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_courses, MAX_NUM_OF_COURSES, false, file, "Course Number");

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

void release_courses(Course *courses, int num_courses)
{
    for (size_t i = 0; i < num_courses; i++)
    {
        Course *current_course = &courses[i];
       
        for (size_t j = 0; j < current_course->num_sections; ++j)
        {
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

void process_courses(Course *courses, int num_courses)
{
    for (size_t i = 0; i < num_courses; i++)
    {
        Course *current_course = &courses[i];
        OutputInfo *out_info = countPassedStudents(current_course);

        printf("%s %d", current_course->course_name, out_info->pass_count);

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

OutputInfo *initOutputInfo(const Course *course)
{
    Student *student = &(course->sections[0][0]);
    int *num_scores = &(course->num_scores[0]);

    OutputInfo *out_info = (OutputInfo *)malloc(sizeof(OutputInfo));

    out_info->pass_count = 0;

    out_info->avg_scores_per_section = (float *)malloc(course->num_sections * sizeof(float));
    
    out_info->student = (Student *)malloc(sizeof(Student));
            
    out_info->student->id = student->id;
    
    out_info->student->lname = (char *)malloc((strlen(student->lname) + 1) * sizeof(char));
    
    strcpy(out_info->student->lname, student->lname);

    out_info->student->scores = (float *)malloc(*num_scores * sizeof(float));

    memcpy(out_info->student->scores, student->scores, course->num_scores[0] * sizeof(float));
    
    out_info->student->std_avg = student->std_avg;

    return out_info;
}

OutputInfo *countPassedStudents(Course *course)
{
    OutputInfo *out_info = initOutputInfo(course);

    for (size_t i = 0; i < course->num_sections; i++)
    {
        float *avg_scores = &(out_info->avg_scores_per_section[i]);
        *avg_scores = 0;

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


void studentCpy(Student *dist, const Student *src, const int *num_scores)
{
    dist->id = src->id;

    dist->lname = (char *)realloc(dist->lname, (strlen(src->lname) + 1) * sizeof(char));

    strcpy(dist->lname, src->lname);

    dist->std_avg = src->std_avg;

    dist->scores = (float *)realloc(dist->scores, *num_scores * sizeof(float));

    memcpy(dist->scores, src->scores, *num_scores * sizeof(float));
}


Student **readSections(FileInfo *file, int students[], int scores[], int num_sections)
{
    Student **sections_and_students = (Student **)malloc(num_sections * sizeof(Student *));

    for (size_t i = 0; i < num_sections; i++)
    {
        int *num_students = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_students, MAX_NUM_OF_STUDENTS_IN_SECTION, false, file, "Student Number");

        int *num_scores = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_scores, MAX_NUM_OF_ASSIGNMENTS_IN_SECTION, false, file, "Assignment Number");

        students[i] = *num_students;
        scores[i] = *num_scores;

        sections_and_students[i] = (Student *)malloc(*num_students * sizeof(Student));

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

            student->scores = (float *)malloc(*num_scores * sizeof(float));

            for (size_t k = 0; k < *num_scores; k++)
            {
                float *score = (float *)StringToNumber(getWord(file), Float);
                testLimits(*score, MAX_SCORE, true, file, "Student Score");

                score_avg += *score;

                *(student->scores + k) = *score;

                free(score);
            }

            student->std_avg = score_avg / *num_scores;
        }

        free(num_students);
        free(num_scores);
    }

    return sections_and_students;
}

Course *readCourses(FileInfo *file, int *num_courses)
{
    Course *courses = (Course *)malloc(sizeof(Course) * (*num_courses));

    for (size_t i = 0; i < *num_courses; i++)
    {
        Course *current_course = &courses[i];

        current_course->course_name = getWord(file);
        testLimits((int)strlen(current_course->course_name), MAX_COURSE_NAME_LENGTH, false, file, "Course Name");

        int *num_sections = (int *)StringToNumber(getWord(file), Integer);
        testLimits(*num_sections, MAX_NUM_OF_SECTIONS, false, file, "Section Number");
        
        current_course->num_sections = *num_sections;
        free(num_sections);

        current_course->num_students = (int *)malloc(current_course->num_sections * sizeof(int));
        current_course->num_scores = (int *)malloc(current_course->num_sections * sizeof(int));

        current_course->sections = readSections(file, current_course->num_students, current_course->num_scores, current_course->num_sections);
    }

    return courses;
}

void *StringToNumber(char *string, DataType data_type)
{
    if (data_type == Integer)
    {
        int *num = (int *)malloc(sizeof(int));
        *num = atoi(string);
        free(string);
        return num;
    }
    else if (data_type == Float)
    {
        float *num = (float *)malloc(sizeof(float));
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

void closeFile(FileInfo *file)
{
    fclose(file->fptr);
    free(file);
}

FileInfo *openFile(const char *filename, const char *mode)
{
    FileInfo *file = (FileInfo *)malloc(sizeof(FileInfo));

    FILE *fptr = fopen(filename, mode);

    file->fptr = fptr;
    file->current_line = 1;

    return file;
}

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

            lineptr = (char *)realloc(lineptr, size);
        }

        lineptr[index++] = ch;
    }

    return lineptr;
}