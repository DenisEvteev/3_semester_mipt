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

#define PATH_TO_DUMP_BSP_TREE "Tests/bsp_trees/pentagon.tex"
#define TRIANGLE_TESTS "Tests/data/triangle.txt"
#define TRIANGLE_RESULTS "Tests/res/.txt"
#define LATEX_REPORT_FILE_PATH "Tests/latex/info.tex"

enum {
    POS_RES_NUMBER = 10,
    POS_NUMBER = 19,
    TYPE_OF_THE_FIRST_POLYGON = 3,
    TYPE_OF_THE_SECOND_POLYGON = 3
};

namespace polygon {

    /*this is a template friend function of class Work_w_Polygon
     * I'm going to use it when i need to have the input from standart input file stream
     * : FILENO_STDIN, besides this operator will fill in two */
    template<typename T>
    class Work_w_Polygon;

    template<typename T>
    std::istream &operator>>(std::istream &in, Work_w_Polygon<T> &object);

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
        using const_polyg_reference = const Work_w_Polygon<coord_type> &;
        using polyg_reference = Work_w_Polygon<coord_type> &;
    private:
        bsp::BSPTree<coord_type> *polygon_1 = nullptr;
        bsp::BSPTree<coord_type> *polygon_2 = nullptr;

        std::list <line_type> common_lines;
        std::string id_file_for_tests;

        void Parse_The_String_With_Coordinates_And_Create_Two_Polygons(const std::string &convenient_work);

    public:

        Work_w_Polygon() = default;

        Work_w_Polygon(vector_coords first_polygon, vector_coords second_polygon);

        Work_w_Polygon(const_polyg_reference copy);

        polyg_reference operator=(const_polyg_reference copy);



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

        friend std::istream &operator>><coord_type>(std::istream &in, Work_w_Polygon<T> &object);


    };

    /*_______________A small help for the users of this application____________
     * If you want to know how does this program works and what kind of steps it produces in case of each function
     * you can call the corresponding function in each class and preamble function below
     * and define in each corresponding file define
     * [DUMP_THE_PROCESS_INFORMATION_IN_LATEX]
     *
     * doing this you can :
     * 1) Look though the input data of coordinates
     * 2) Look at the bsp tree representation of polygons as a digraphs created using dot language
     * 3) Look at the answer of the calculated area of intersection of polygons
     * 4) Look at the common line segments which form the common polygon of intersection
     * 5) Look at the developer contacts
     *
     *
     *
     * ______This function takes a string which represents the number of file to dump the whole
     * information about the process*/

    void DumpPreambleInLatex(std::ofstream &out);

    void File_Is_Not_Opened();
}


#endif //COUNT_INTERSECTION_AREA_OF_TRIANGLE_WORK_W_POLYGON_H
