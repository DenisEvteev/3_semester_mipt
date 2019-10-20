#include <iostream>
#include "Work_w_Polygon.hpp"
#include "BSPTree.hpp"
#include <vector>
#include "line_2d.hpp"


using namespace bsp;
using namespace polygon;

void test_lines_intersection(const line_2d<float> &line1, const line_2d<float> &line2);

int main() {
//    //i'm going to write different tests for checking function lines_intersection
//    //-------------------------------------------------------------------------------------------------------------------//
//    line_2d<float> line1({2,2}, {2, 5});
//    line_2d<float> line2({2,2}, {4,2});
//    test_lines_intersection(line1, line2);
//    //--------------------------------------------------------------------------------------------------------------------------------//
//    line_2d<float> line3({2, 1}, {2, 6});
//    line_2d<float> line4({1, 6}, {2, 6});
//    test_lines_intersection(line3, line4);
//    //----------------------------------------------------------------------------------------//
//    line_2d<float> line5({6, 3}, {4, 6});
//    line_2d<float> line6({2, 1}, {6, 5});
//    test_lines_intersection(line5, line6);
//    //-----------------------------------------------------------------------------------------//
//    line_2d<float> line7({3, 1}, {3, 5});
//    line_2d<float> line8({1, 3}, {5, 3});
//    test_lines_intersection(line7, line8);
//    //--------------------------------------------------------------------------------------//
//    line_2d<float> line9({1, 1}, {2, 3});
//    line_2d<float> line10({1, 2}, {2, 2});
//    test_lines_intersection(line7, line10);
//    //-------------------------------------------------------------------------------//



// test of creating bsp tree for triangle
    //std::vector<point_2d<float>> points1 = {{4, 6}, {1, 2}, {9, 1}};
    //test of creating bsp tree for polygon with 4 edges

    //std::vector<point_2d<float>> points3 = {{6, 2}, {7, 4}, {4, 7}, {1, 4}};

//    std::vector<point_2d<float>> points2 = {{1, 3}, {10, 3}, {4, 7}};
//    std::vector<point_2d<float>> points1 = {{2, 8}, {6, 2}, {9, 7}};
//
//    std::vector<point_2d<float>> points2 = {{2, 8}, {6, 1}, {10, 8}};
//    std::vector<point_2d<float>> points1 = {{1, 3}, {11, 3}, {5, 10}};


//    std::vector<point_2d<float>> points2 = {{1, 1}, {3, 1}, {1, 6}};
//    std::vector<point_2d<float>> points1 = {{2, 1}, {3, 2}, {1, 3}};

//    std::vector<point_2d<float>> points2 = {{1, 0}, {3, 0}, {2, 1}};
//    std::vector<point_2d<float>> points1 = {{1, 1}, {3, 1}, {2, 3}};



//    std::vector<point_2d<float>> points2 = {{2, 2}, {6, 2}, {2, 8}};
//    std::vector<point_2d<float>> points1 = {{2, 3}, {4, 3}, {4, 5}};

//std::vector<point_2d<float>> points2 = {{0,0}, {3, 0}, {0, 3}};
//std::vector<point_2d<float>> points1 = {{2, 3}, {-1, 0}, {0, -1}};

//    std::vector<point_2d<float>> points2 = {{0,0}, {3, 0}, {0, 3}};
//    std::vector<point_2d<float>> points1 = {{-1, 2}, {2, -1}, {2, 2}};
//
//    std::vector<point_2d<float>> points2 = {{2, 7}, {2, 3}, {6, 3}};
//    std::vector<point_2d<float>> points1 = {{4,5}, {0, 3}, {4, 0}};

//    std::vector<point_2d<float>> points2 = {{0, 0}, {8, 0}, {3, 5}};
//    std::vector<point_2d<float>> points1 = {{2,0}, {7, 1}, {4, 4}};

//    std::vector<point_2d<float>> points2 = {{0, 0}, {3, 0}, {3, 6}};
//    std::vector<point_2d<float>> points1 = {{1,2}, {3, 2}, {3, 6}};

//    std::vector<point_2d<float>> points2 = {{0, 0}, {8, 0}, {4, 4}};
//    std::vector<point_2d<float>> points1 = {{2,2}, {6, 2}, {2, 6}};

    std::vector <point_2d<float>> points2 = {{0, 0},
                                             {6, 0},
                                             {3, 6}};
    std::vector <point_2d<float>> points1 = {{0, 4},
                                             {6, -4},
                                             {6, 4}};

//    std::vector<point_2d<float>> points2 = {{0, 0}, {6, 0}, {3, 3}};
//    std::vector<point_2d<float>> points1 = {{3,0}, {5, -2}, {5, 3}};

    Work_w_Polygon<float> triangle_intersection(points1, points2);

    triangle_intersection.intersect_polygons();

    float area_intersect = triangle_intersection.Area_Trapezoids();

    std::cout << area_intersect << std::endl;

//    BSPTree<float> triangle1(points1);
//    BSPTree<float> triangle2(points2);
//
//
//
//    triangle1.Make_Tree();
//    triangle2.Make_Tree();
//
//    std::vector<line_2d<float>> edges_new_polygon;
//    triangle1.Polygon_Polygon_Intersection(edges_new_polygon, triangle2.get_root());
//    triangle2.Polygon_Polygon_Intersection(edges_new_polygon, triangle1.get_root());



//std::vector<point_2d<float>> coords_rectangle = {{2,12}, {10, 7}, {11, 11}, {4, 15}};
//BSPTree<float> rectangle(coords_rectangle);
//rectangle.Make_Tree();

    return 0;
}

//void test_lines_intersection(const line_2d<float>& line1, const line_2d<float>& line2){
//    int type = 0;
//    point_2d<float>* common_pt = line1.lines_intersection(line2, &type);
//
//    std::cout << "type intersec : " << type << std::endl;
//    if(common_pt){
//        std::cout << "x_coord : " << common_pt->x_ << " ___ y_coord is : " << common_pt->y_ << std::endl;
//
//        std::cout << std::endl;
//        delete common_pt;
//    }
//}