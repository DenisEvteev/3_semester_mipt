/*One interesting thing about my project that if you want to know how do most of the pivotal functions work
 * than you can define DUMP_THE_PROCESS_INFORMATION_IN_LATEX in files and you will get the name.tex file with
 * full correct information to compile into name.pdf format
 * but now this dumping work only when the input string of coordinates are taken from the operator >> for the Work_w_Polygons*/

#include <iostream>
#include <vector>
#include <cassert>


#include "Work_w_Polygon.hpp"
#include "BSPTree.hpp"
#include "line_2d.hpp"


void MyNewHandler();

using namespace bsp;
using namespace polygon;




/*if you want to get the input data from file of my type ( the examples are represented in the catalog Tests in folder
 * data) then you must define DATA_FROM_FILE otherwise the data will be requested from standart input file stream STDOUT_FILENO*/

//#define DATA_FROM_FILE


int main() {

    /*I've changed the pointer to the function which will be called in case
     * when the operating system cannot manage with allocating more memory for this process*/
    std::set_new_handler(MyNewHandler);

    Work_w_Polygon<float> triangle_intersection;


    try {
#ifndef DATA_FROM_FILE
        std::cin >> triangle_intersection;
#else
        triangle_intersection.Polygons_From_File();
#endif
    } catch (std::invalid_argument &ex) {
        ex.what();
        exit(EXIT_FAILURE);
    } catch (std::out_of_range &ex) {
        ex.what();
        exit(EXIT_FAILURE);
    }

    triangle_intersection.intersect_polygons();

    /*Counting the area of intersection
     * This method will print the result value to the corresponding file if PRINT_RESULT_IN_FILE will be defined
     * conversly it will return only the the result as a copy*/
    float area_intersect = triangle_intersection.Area_Trapezoids();

    assert(area_intersect >= 0);

    std::cout << area_intersect << std::endl;

    return 0;
}

void MyNewHandler() {
    std::cout << "operator new cannot allocate more memory\n";
    std::abort();
}