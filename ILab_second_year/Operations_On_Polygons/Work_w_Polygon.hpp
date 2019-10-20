//
// Created by denis on 13.10.19.
//

#include "Work_w_Polygon.h"

using namespace polygon;


template<typename T>
Work_w_Polygon<T>::Work_w_Polygon(vector_coords first_polygon, vector_coords second_polygon) {
    polygon_1 = new bsp::BSPTree<T>(first_polygon);
    polygon_2 = new bsp::BSPTree<T>(second_polygon);

    polygon_1->Make_Tree();
    polygon_2->Make_Tree();

}

template<typename T>
Work_w_Polygon<T>::~Work_w_Polygon() {

    polygon_1->Clear_Bsp_Tree(polygon_1->get_root());
    polygon_2->Clear_Bsp_Tree(polygon_2->get_root());

    delete polygon_1;
    delete polygon_2;

}

template<typename T>
typename Work_w_Polygon<T>::coord_type Work_w_Polygon<T>::Area_Trapezoids() const {

    if (common_lines.size() == 0)
        return WITHOUT_INTERSECTON;

    coord_type double_neg_area = 0;

    for (c_iter it = common_lines.cbegin(); it != common_lines.cend(); ++it)
        double_neg_area += (it->pt1_.y_ + it->pt2_.y_) * (it->pt2_.x_ - it->pt1_.x_);


    coord_type pos_area = (1.0 / 2.0) * double_neg_area;
    return std::fabs(pos_area);
}

template<typename T>
void Work_w_Polygon<T>::intersect_polygons() {
    polygon_1->Polygon_Polygon_Intersection(common_lines, polygon_2->get_root());
    polygon_2->Polygon_Polygon_Intersection(common_lines, polygon_1->get_root());


    /*This functions will change places of some line segments to produce
     * counterclockwise direction of polygon in intersection*/
    MakeCounterClockwiseOrder();
}


/*I'm going to use this function in loop to produce well ordered list of line_segments of
 * new polygon in intersection of two other polygons*/
template<typename T>
void Work_w_Polygon<T>::MakeCounterClockwiseOrder() {
    if (common_lines.size() == 0)
        return;

    iter next_line = common_lines.begin();
    ++next_line;

    for (iter it = common_lines.begin(); it != common_lines.end() && next_line != common_lines.end(); ++it) {
        if (next_line->pt1_ == it->pt2_) {
            ++next_line;
            continue;
        } else {
            iter it_find = next_line;
            ++it_find;

            for (it_find; it_find != common_lines.end(); ++it_find) {
                if (it_find->pt1_ == it->pt2_) {
                    common_lines.insert(next_line, *it_find);
                    common_lines.erase(it_find);
                    break;
                }
            }

        }

    }
}