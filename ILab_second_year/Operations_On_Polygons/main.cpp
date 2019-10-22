#include <iostream>
#include "Work_w_Polygon.hpp"
#include "BSPTree.hpp"
#include <vector>
#include "line_2d.hpp"


void MyNewHandler();

using namespace bsp;
using namespace polygon;


int main() {

    std::set_new_handler(MyNewHandler);

    Work_w_Polygon<float> triangle_intersection;

    triangle_intersection.Polygons_From_File();

    triangle_intersection.intersect_polygons();

    float area_intersect = triangle_intersection.Area_Trapezoids();

    std::cout << area_intersect << std::endl;

    return 0;
}

void MyNewHandler() {
    std::cout << "operator new cannot allocate more memory\n";
    std::abort();
}