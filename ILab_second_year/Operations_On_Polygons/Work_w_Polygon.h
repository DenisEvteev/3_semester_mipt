//
// Created by denis on 13.10.19.
//

#ifndef COUNT_INTERSECTION_AREA_OF_TRIANGLE_WORK_W_POLYGON_H
#define COUNT_INTERSECTION_AREA_OF_TRIANGLE_WORK_W_POLYGON_H

#include "BSPTree.h"
#include <list>
#include <algorithm>


#define WITHOUT_INTERSECTON 0

namespace polygon {
    template<typename T>
    class Work_w_Polygon {
        using coord_type = T;
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

    public:

        Work_w_Polygon() = default;

        Work_w_Polygon(vector_coords first_polygon, vector_coords second_polygon);

        void intersect_polygons();

        void MakeCounterClockwiseOrder();


        /*This function calculates the area of each 2d polygon without self intersections.
         * It uses the algorithm of calculating the difference of two areas which are sum of areas of trapezoids*/

        coord_type Area_Trapezoids() const;

        ~Work_w_Polygon();


    };


}


#endif //COUNT_INTERSECTION_AREA_OF_TRIANGLE_WORK_W_POLYGON_H
