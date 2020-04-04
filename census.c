/* Dillon Benson
 * This program reads census data from a file.
 * The user enters a year between 1790 and 2010.
 * Then, the program finds the population of New Jersey
 * for the year entered by the user using linear
 * interpolation.
 */

// include appropriate headers
#include <stdio.h>
#include <stdlib.h>

// define some constants
#define DATA_FILE "./njpopulation.dat"
#define EARLIEST_YEAR 1790
#define LATEST_YEAR 2010
#define BUFFER_SIZE 64
#define DECADES 23
#define DEBUG 0
#define NO_OP

// define bool type
typedef enum { false, true } bool;

// check for NULL definition, assigning 0 if not already defined
#ifndef NULL
  #define NULL 0
#endif

// show console logs when DEBUG is true
#if DEBUG
  #define LOG(format, ...) fprintf(stderr, format, __VA_ARGS__)
#else
  #define LOG(_, ...) NO_OP
#endif

// define a struct to contain information needed for linear interpolation
typedef struct {
  int population_year;
  int lower_decade;
  int upper_decade;
  float lower_decade_population;
  float upper_decade_population;
} InterpolationData;

// function prototypes
void input_loop();
int lower_decade_index(int, int, int[DECADES]);
float interpolate(InterpolationData*);
void print_results(int, float, bool);

// main driver
int main(int argc, char **argv) {
  // declare and initialize a file pointer for reading from the data file
  FILE *fp = fopen(DATA_FILE, "r");
  // create an integer array to hold every decade year number from 1790 to 2010
  int years[DECADES];
  // create a float array to hold every decades population from 1790 to 2010
  float populations[DECADES];
  // index used to read / write population info
  int i = 0;

  // read the 23 decades information from the data file and store in arrays declared above
  while(fscanf(fp, "%d %f", &years[i], &populations[i]) && i < DECADES) {
    LOG("%d %f\n", years[i], populations[i]);
    i++;
  }

  // close the datafile
  fclose(fp);

  // prompts user to enter a year and prints the corresponding population count
  input_loop(years, populations);

  // terminate program successfully
  return 0;
}

// prompt user to enter a year and print the corresponding population
// this process stops when the user enters 0 for the year
// @years - the year numbers read from data file (first column)
// @populations - the populations read from data file (second column)
void input_loop(int years[DECADES], float populations[DECADES]) {
  // stores the year entered by the user (if it was valid and in range)
  int population_year;
  // the actual string entered by the user that will be converted to an integer for the year
  char input[BUFFER_SIZE];
  // year difference from lower decade, e.g: 1792 = 2, 1993 = 3
  int decade_offset;
  // stores the index of the lower decade from the years array
  int index;

  // prompt user to enter a year and store the input as a string in input
  printf("What year would you like to find or approximate the population of New Jersey for (>= %d and <= %d): ", EARLIEST_YEAR, LATEST_YEAR);
  scanf("%s", input);

  // check if user entered 0 by itself, if so break out of input loop
  if(input[0] == '0' && (input[1] == '\0' || input[1] == '\n'))
    return;

  // try to convert input from user into an integer
  // if this fails, tell the user the year is not valid and recurse
  if(!(population_year = atoi(input))) {
    printf("Please enter a valid number for the year (you entered: %s)\n", input);
    return input_loop(years, populations);
  }

  // check if year entered by user is in range of 1790 to 2010
  if(population_year < EARLIEST_YEAR || population_year > LATEST_YEAR) {
    printf("Please enter a year >= %d and <= %d (you entered: %d)\n", EARLIEST_YEAR, LATEST_YEAR, population_year);
    return input_loop(years, populations);
  }

  // store difference from lower decade
  decade_offset = population_year % 10;
  // calculate index of lower decade
  index = lower_decade_index(population_year, decade_offset, years);

  // if there is no difference, show the actual population for that year.
  // otherwise, calculate the population using linear interpolation and
  // show an approximate population of that year entered
  if(!decade_offset) {
    LOG("Interpolation NOT NEEDED here.\n", NULL);

    // print year and exact population
    print_results(population_year, populations[index], true);

    // recurse
    return input_loop(years, populations);
  } else {
    LOG("Interpolation NEEDED here.\n", NULL);

    // initialize InterpolationData struct for sake of reducing the number
    // of arguments passed to the interpolate function.
    // this will be allocated on the heap using malloc
    InterpolationData *interpolation_data = (InterpolationData*)malloc(sizeof(InterpolationData));
    interpolation_data->population_year = population_year;
    interpolation_data->lower_decade = years[index];
    interpolation_data->upper_decade = years[index + 1];
    interpolation_data->lower_decade_population = populations[index];
    interpolation_data->upper_decade_population = populations[index + 1];

    // print year and approximate population
    print_results(population_year, interpolate(interpolation_data), false);

    // free the struct created earlier
    free(interpolation_data);

    // recurse
    return input_loop(years, populations);
  }
}

// calculates the index of the lower decade e.g: 1792 = 0, 1804 = 1, 1819 = 2, 1820 = 3, ...
int lower_decade_index(int population_year, int decade_offset, int years[DECADES]) {
  int lower_decade = population_year - decade_offset;

  for(int i = 0; i < DECADES; i++)
    if(lower_decade == years[i])
      return i;

  // suppress compiler warning about unreachable return
  __builtin_unreachable();
}

// performs linear interpolation to approximate the population
float interpolate(InterpolationData *interpolation_data) {
  float y0 = (float)interpolation_data->lower_decade_population;
  float y1 = (float)interpolation_data->upper_decade_population;
  float x = (float)interpolation_data->population_year;
  float x0 = (float)interpolation_data->lower_decade;
  float x1 = (float)interpolation_data->upper_decade;

  return y0 + ((y1 - y0) * (x - x0)) / (x1 - x0);
}

// function to print the year and population count (exactly or approximately)
void print_results(int population_year, float population_count, bool exact) {
  char *approx_str = exact ? "" : "(approximately)";
  printf("\nYear: %d\nPopulation: %.2f %s\n\n", population_year, population_count, approx_str);
}
