//
// Created by denis on 13.10.19.
//

#include "Work_w_Polygon.h"

using namespace polygon;


template<typename T>
Work_w_Polygon<T>::Work_w_Polygon(vector_coords first_polygon, vector_coords second_polygon) {
    Produce_Bsp_Trees(first_polygon, second_polygon);
}

template<typename T>
Work_w_Polygon<T>::~Work_w_Polygon() {

    polygon_1->Clear_Bsp_Tree(polygon_1->get_root());
    polygon_2->Clear_Bsp_Tree(polygon_2->get_root());

    delete polygon_1;
    delete polygon_2;

}

template<typename T>
void Work_w_Polygon<T>::Produce_Bsp_Trees(vector_coords first_polygon, vector_coords second_polygon) {
    polygon_1 = new bsp::BSPTree<coord_type>(first_polygon);
    polygon_2 = new bsp::BSPTree<coord_type>(second_polygon);

    polygon_1->Make_Tree();
    polygon_2->Make_Tree();
}

template<typename T>
void Work_w_Polygon<T>::Polygons_From_File() {

    std::cout << "Enter the number of test for processing : " << std::endl;
    std::cin >> id_file_for_tests;

    std::string path_to_test(TRIANGLE_TESTS);
    path_to_test.insert(POS_NUMBER, id_file_for_tests);

    std::ifstream file_with_data;
    file_with_data.open(path_to_test);

    if (!file_with_data.is_open()) {
        std::cout << "ERROR in opening file " << path_to_test << " line with error is : " << __LINE__ <<
                  " function is : " << __PRETTY_FUNCTION__ << std::endl;

        exit(EXIT_FAILURE);
    }

    file_with_data.seekg(0, std::ios_base::end);
    size_t size_bytes = file_with_data.tellg();
    file_with_data.seekg(0, std::ios_base::beg);

    //put all the data in buf
    char *buf = new char[size_bytes + 1]{'\0'};
    file_with_data.read(buf, size_bytes + 1);
    std::string convenient_work(buf);

    convenient_work.push_back(' ');
    //two polygons for creating trees based on them
    std::vector<point_type> first_polygon;
    std::vector<point_type> second_polygon;

    //represents the number of vector which we must fill with coordinates
    int number_vector = 0;
    int another_begin_set_coordinates = 0;

    size_t ip = 0;
    while (ip < size_bytes) {

        if (convenient_work[ip] == '#') {
            ip = convenient_work.find('\n', ip);
            ++ip;
            //this value show us when the second set of coordinates starts and we will use it to change ip where space
            //appears further than the end of string of the first set of coordinates
            another_begin_set_coordinates = convenient_work.find('\n', ip);
            continue;
        } else if (convenient_work[ip] == ' ') {
            ++ip;
            continue;
        } else if (convenient_work[ip] == '\n') {
            ++number_vector;
            ++ip;
            another_begin_set_coordinates = convenient_work.size() + 1;
            continue;
        }

            //the case when we have the float value in string as a current position
        else {
            //the space between each coordinates must be exactly only otherwise the behaviour of the program will be very bad
            //i will probably improve this point but not now
            size_t next_space = convenient_work.find(' ', ip);
            coord_type x_coord = std::stof(convenient_work.substr(ip, next_space));
            ip = next_space + 1;
            next_space = convenient_work.find(' ', ip);
            coord_type y_coord = std::stof(convenient_work.substr(ip, next_space));

            if (next_space > another_begin_set_coordinates) {
                ip = another_begin_set_coordinates;
            } else {
                ip = next_space;
            }

            !number_vector ? first_polygon.push_back(point_type(x_coord, y_coord)) :
            second_polygon.push_back(point_type(x_coord, y_coord));
        }


    }

    Produce_Bsp_Trees(first_polygon, second_polygon);

    delete[] buf;
    file_with_data.close();

}

//#define PRINT_RESULT_IN_FILE

template<typename T>
typename Work_w_Polygon<T>::coord_type Work_w_Polygon<T>::Area_Trapezoids() const {
    coord_type double_neg_area = 0;

    for (c_iter it = common_lines.cbegin(); it != common_lines.cend(); ++it)
        double_neg_area += (it->pt1_.y_ + it->pt2_.y_) * (it->pt2_.x_ - it->pt1_.x_);


    coord_type pos_area = (1.0 / 2.0) * double_neg_area;
    coord_type area = std::fabs(pos_area);

#ifdef PRINT_RESULT_IN_FILE
    std::ofstream file_with_result;
    std::string path_to_file(TRIANGLE_RESULTS);
    path_to_file.insert(POS_RES_NUMBER, id_file_for_tests);

    file_with_result.open(path_to_file);
    if (!file_with_result.is_open()) {
        std::cout << "File : " << path_to_file << " cannot be created " << std::endl;
        exit(EXIT_FAILURE);
    }

    file_with_result << area;
    file_with_result.close();
#endif

    return area;
}

#undef PRINT_RESULT_IN_FILE

template<typename T>
void Work_w_Polygon<T>::intersect_polygons() {

    polygon_1->Polygon_Polygon_Intersection(common_lines, polygon_2->get_root());
    polygon_2->Polygon_Polygon_Intersection(common_lines, polygon_1->get_root());


    /*This functions will change places of some line segments to produce
     * counterclockwise direction of polygon in intersection
     * I mean that this function will change place of some edges in list of final edges of new polygon
     * to make it be in exactly counterclockwise direction :
     * each begin of the edge starts with the end of the previous edge except the first one due to it has the start
     * in the end of last edge in the list*/
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

    for (iter it = common_lines.begin(); it != common_lines.end() && next_line != common_lines.end();) {

        if (next_line->pt1_ == it->pt2_) {
            ++next_line;
            ++it;
            continue;

        } else {
            iter it_find = next_line;
            ++it_find;

            for (it_find; it_find != common_lines.end();) {
                if (it_find->pt1_ == it->pt2_) {
                    it = common_lines.insert(next_line, *it_find);
                    common_lines.erase(it_find);
                    next_line = it;
                    ++next_line;
                    break;
                }

                ++it_find;
                //the case of abnormal situation when two almost equal point
                // turned out not equal due to calculation intersection point coordinates
                //we are going to check if any of
                if (it_find == common_lines.end()) {
                    ++it;
                    ++next_line;
                    break;
                }

            }

        }

    }
}

template<typename T>
std::istream &polygon::operator>>(std::istream &in, Work_w_Polygon<T> &object) {
    //here we will read the data from input file
    return in;

}