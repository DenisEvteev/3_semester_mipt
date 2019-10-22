//
// Created by denis on 13.10.19.
//

#ifndef COUNT_INTERSECTION_AREA_OF_TRIANGLE_WORK_W_POLYGON_H
#define COUNT_INTERSECTION_AREA_OF_TRIANGLE_WORK_W_POLYGON_H

#include "BSPTree.h"
#include <list>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>


#define TRIANGLE_TESTS "data/triangle.txt"
#define TRIANGLE_RESULTS "res/.txt"

#define POS_RES_NUMBER 4
#define POS_NUMBER 13


#define WITHOUT_INTERSECTON 0

namespace polygon {
    template<typename T>
    class Work_w_Polygon {
        using coord_type = T;
        using coord_ptr = T *;
        using point_type = point_2d<T>;
        using line_type = line_2d<T>;
        using const_line_reference = const line_2d<coord_type> &;
        using const_point_reference = const point_2d<T> &;
        using vector_coords = const std::vector <point_type> &;
        using iter = typename std::list<line_type>::iterator;
        using c_iter = typename std::list<line_type>::const_iterator;
    private:
        bsp::BSPTree<coord_type> *polygon_1 = nullptr;
        bsp::BSPTree<coord_type> *polygon_2 = nullptr;

        std::list <line_type> common_lines;
        std::string id_file_for_tests;

    public:

        Work_w_Polygon() = default;

        Work_w_Polygon(vector_coords first_polygon, vector_coords second_polygon);

        void intersect_polygons();

        /*This method is intended to take input coordinates from file and creating two bsp trees from it
         * It has one disadvantage : only float values can be parsed for the moment
         * you must enter the number of test to calculate the intersection of polygons and it will parce fill two vectors
         * of coordinates of polygons*/

        void Polygons_From_File();

        void MakeCounterClockwiseOrder();

        void Produce_Bsp_Trees(vector_coords first_polygon, vector_coords second_polygon);

        /*This function calculates the area of each 2d polygon without self intersections.
         * It uses the algorithm of calculating the difference of two areas which are sum of areas of trapezoids*/

        /*This function at the moment put the result of calculating area in the file with the ordinal number the same
         * as the input file with data*/
        coord_type Area_Trapezoids() const;


        ~Work_w_Polygon();


    };


}


#endif //COUNT_INTERSECTION_AREA_OF_TRIANGLE_WORK_W_POLYGON_H
