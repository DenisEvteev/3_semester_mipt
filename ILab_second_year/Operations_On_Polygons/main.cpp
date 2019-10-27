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

    std::set_new_handler(MyNewHandler);

    Work_w_Polygon<float> triangle_intersection;

#ifndef DATA_FROM_FILE
    std::cin >> triangle_intersection;
#else
    triangle_intersection.Polygons_From_File();
#endif


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