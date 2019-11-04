//
// Created by denis on 13.10.19.
//

#include "Work_w_Polygon.h"

//#define DUMP_THE_PROCESS_INFORMATION_IN_LATEX

using namespace polygon;


template<typename T>
Work_w_Polygon<T>::Work_w_Polygon(vector_coords first_polygon, vector_coords second_polygon) {
    Produce_Bsp_Trees(first_polygon, second_polygon);
}

//impementation of copy constructor and assignment operator
template<typename T>
Work_w_Polygon<T>::Work_w_Polygon(const_polyg_reference copy) :
        polygon_1(new bsp::BSPTree<coord_type>(*copy.polygon_1)),
        polygon_2(new bsp::BSPTree<coord_type>(*copy.polygon_2)),
        common_lines(copy.common_lines),
        id_file_for_tests(copy.id_file_for_tests) {}


template<typename T>
typename Work_w_Polygon<T>::polyg_reference Work_w_Polygon<T>::operator=(const_polyg_reference copy) {
    if (this == &copy)
        return *this;
    common_lines = copy.common_lines;
    id_file_for_tests = copy.id_file_for_tests;

    /*here the destructors both of polygon_1 and polygon_2 will work and
     * the allocated memory will be freed in a exactly right way
     **/
    delete polygon_1;
    delete polygon_2;

    polygon_1 = new bsp::BSPTree<coord_type>(*copy.polygon_1);
    polygon_2 = new bsp::BSPTree<coord_type>(*copy.polygon_2);

    return *this;
}
//-----------------------------------------------------------------------------------------------------//


template<typename T>
Work_w_Polygon<T>::~Work_w_Polygon() {

    delete polygon_1;

    delete polygon_2;

}

template<typename T>
void Work_w_Polygon<T>::Produce_Bsp_Trees(vector_coords first_polygon, vector_coords second_polygon) {

    polygon_1 = new bsp::BSPTree<coord_type>(first_polygon);

    polygon_2 = new bsp::BSPTree<coord_type>(second_polygon);
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
    char *buf = new char[size_bytes + 1];
    memset(buf, '\0', size_bytes + 1);

    file_with_data.read(buf, size_bytes);
    std::string convenient_work(buf);

    Parse_The_String_With_Coordinates_And_Create_Two_Polygons(convenient_work);

    delete[] buf;
    file_with_data.close();

}

/*At the moment i can parce with string which contain only : ['\n', '-', float values,  ' ', '#']
 * If the string contain some different symbols than the behavior is undefined
 * It the the strictest requirements to give in input the strings of the exact form !!!!!!!!!!!!!!! */

template<typename T>
void Work_w_Polygon<T>::Parse_The_String_With_Coordinates_And_Create_Two_Polygons(const std::string &convenient_work) {
    //two polygons for creating trees based on them
    std::vector<point_type> first_polygon;
    std::vector<point_type> second_polygon;

    /*This const static string represent the sequence of bad symbols which
     * can appear in the input string of data and which we want to skip during parcing*/
    const static std::string bad_characters("#\n ");

    size_t ip = 0;
    while (ip < convenient_work.size()) {

        if (convenient_work[ip] == '#') {
            ip = convenient_work.find('\n', ip);
            ++ip;
            continue;
        } else if (convenient_work[ip] == ' ' || convenient_work[ip] == '\n') {
            ++ip;
            continue;
        }


        else {
            /*___________________ IF we won't be able to find the second coordinate in the _________________
            * string then immediate error will be ocurred */

            //find the first bad character
            size_t the_end_of_the_value = convenient_work.find_first_of(bad_characters, ip);
            assert(the_end_of_the_value != std::string::npos);

            //find the start of the second coordinate of the edge

            size_t next_space = convenient_work.find_first_not_of(bad_characters, the_end_of_the_value + 1);
            assert(next_space != std::string::npos);


            //take the first coordinate
            coord_type x_coord = std::stof(convenient_work.substr(ip, the_end_of_the_value));
            ip = next_space;
            //now i am at the next value

            the_end_of_the_value = convenient_work.find_first_of(bad_characters, ip);

            //take the second coordinate
            coord_type y_coord = std::stof(convenient_work.substr(ip, the_end_of_the_value));

            first_polygon.size() < TYPE_OF_THE_FIRST_POLYGON ? first_polygon.push_back(point_type(x_coord, y_coord))
                                                             : second_polygon.push_back(point_type(x_coord, y_coord));
            ip = the_end_of_the_value;

        }
    }

    assert(first_polygon.size() == TYPE_OF_THE_FIRST_POLYGON && second_polygon.size() == TYPE_OF_THE_SECOND_POLYGON);

    Produce_Bsp_Trees(first_polygon, second_polygon);

}
//------------------------Helper functions--------------------------------------------------//

void polygon::File_Is_Not_Opened() {
    std::cout << "The file isn't opened" << std::endl;
    std::cout << "Error is in the " << __FILE__ << std::endl;
    std::cout << "ERROR in " << __LINE__ << " line" << std::endl;
    exit(EXIT_FAILURE);
}


void polygon::DumpPreambleInLatex(std::ofstream &out) {

    if (!out.is_open()) {
        File_Is_Not_Opened();
    }

    out << "\\documentclass[a4paper,12pt]{article}\n";

    out << "\\usepackage[T2A]{fontenc}\n"
           "\\usepackage[utf8]{inputenc}\n"
           "\\usepackage[english,russian]{babel}\n"
           "\\usepackage[pdf]{graphviz}\n"
           "\\usepackage{graphicx, xcolor}\n";

    out << "\\title{Finding Intersection area\\\\The whole report}\n"
           "\\author{Evteev Denis}\n"
           "\\date{\\today{}}\n";

    out << "\\begin{document}\n";
    out << "\\maketitle\n\n";

}

//----------------------------------------------------------------------------------------//
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

#ifdef DUMP_THE_PROCESS_INFORMATION_IN_LATEX
    std::ofstream file_dump;
    file_dump.open(LATEX_REPORT_FILE_PATH, std::ios::app);
    if (!file_dump.is_open())
        File_Is_Not_Opened();

    file_dump << "\\begin{center}\n";
    file_dump << "\\fbox{\\Large{\\textbf{intersection area is equal to :  " << area << "}}}\n\n";
    file_dump << "\\end{center}\n";
    file_dump << "\\end{document}\n\n";
    file_dump.close();
#endif

    return area;
}

//#undef PRINT_RESULT_IN_FILE

template<typename T>
void Work_w_Polygon<T>::intersect_polygons() {

    /*This line represent finding the lines of polygon_2 which lie within the polygon_1*/
    polygon_1->Polygon_Polygon_Intersection(common_lines, polygon_2->get_root());

    /*This part of code will represent the lines of polygon_2 which lie within polygon_1*/
#ifdef DUMP_THE_PROCESS_INFORMATION_IN_LATEX
    std::ofstream file_dump;
    file_dump.open(LATEX_REPORT_FILE_PATH, std::ios::app);
    if (!file_dump.is_open())
        File_Is_Not_Opened();

    file_dump << "\\section{Finding common lines :}\n\n";
    file_dump << "\\begin{itemize}\n";
    file_dump << "\\item\\noindent\\textbf{The second polygon's line segments that lie within the first one}\\\\\n";
    bsp::Print_Edges_From_List<coord_type>(common_lines.cbegin(), common_lines.cend(), file_dump,
                                           std::string("secondinfirst"));
    auto it = common_lines.cend();
    --it;

#endif

    /*This line represent finding lines of polygon_1 which lie within polygon_2*/
    polygon_2->Polygon_Polygon_Intersection(common_lines, polygon_1->get_root());


#ifdef DUMP_THE_PROCESS_INFORMATION_IN_LATEX

    ++it;
    file_dump << "\\noindent\\item\\textbf{The first polygon's line segments that lie within the second one}\\\\\n";
    bsp::Print_Edges_From_List<coord_type>(it, common_lines.cend(), file_dump, std::string("firstinsecond"));

#endif


    /*This functions will change places of some line segments to produce
     * counterclockwise direction of polygon in intersection
     * I mean that this function will change place of some edges in list of final edges of new polygon
     * to make it be in exactly counterclockwise direction :
     * each begin of the edge starts with the end of the previous edge except the first one due to it has the start
     * in the end of last edge in the list*/
    MakeCounterClockwiseOrder();

#ifdef DUMP_THE_PROCESS_INFORMATION_IN_LATEX

    file_dump << "\\noindent\\item\\textbf{Common line segments in counterclockwise order}\\\\\n";
    bsp::Print_Edges_From_List<coord_type>(common_lines.cbegin(), common_lines.cend(), file_dump, std::string("end"));
    file_dump << "\\end{itemize}\n";
    file_dump.close();

#endif
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

            for (; it_find != common_lines.end();) {
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

/*this method suppose that the user will give twelve float values to create two triangles for
 * finding their intersection area
 * And if the number of coordinates is bad then this function will return an error message
 * */



/*The first dump in latex will start with the input data, so initially we
 * call function connected with latex in here*/
template<typename T>
std::istream &polygon::operator>>(std::istream &in, Work_w_Polygon<T> &object) {
    std::string string_with_input_data_coordinates;
    //request for data from stdin
    std::getline(std::cin, string_with_input_data_coordinates);


#ifdef DUMP_THE_PROCESS_INFORMATION_IN_LATEX
    std::ofstream file_dump(PATH_TO_DUMP_BSP_TREE);
    if (!file_dump.is_open()) {
        File_Is_Not_Opened();
    }

    DumpPreambleInLatex(file_dump);
    file_dump << "\\textbf{Input data :} " << "\\fbox{" <<
              string_with_input_data_coordinates << "}\n";
    file_dump.close();
#endif

    //this call will create two bsp trees for polygons
    object.Parse_The_String_With_Coordinates_And_Create_Two_Polygons(string_with_input_data_coordinates);
    return in;
}


#undef DUMP_THE_PROCESS_INFORMATION_IN_LATEX