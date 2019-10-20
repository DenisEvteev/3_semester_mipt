//
// Created by denis on 13.10.19.
//

#ifndef COUNT_INTERSECTION_AREA_OF_TRIANGLE_LINE_2D_H
#define COUNT_INTERSECTION_AREA_OF_TRIANGLE_LINE_2D_H


#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <limits>

#define NUL_LIMIT 1e-4

#define intersect(type) static_cast<int>(Relationship_Type::type)

enum class Relationship_Type : int {
    Without_Crossing = 0,
    Positive,
    Negative,
    Crossing,
    One_Point,

    //different type of coincedence of two line segments
            Full_Inclusion_In_Spliting_Line,
    Full_Inclusion_Of_Spliting_Line,
    Spliting_Line_Above,
    Spliting_Line_Below

};

template<typename T>
class point_2d;

template<typename T>
std::istream &operator>>(std::istream &in, point_2d<T> &coords);


//functor object due to it has an operator()
template<typename T>
class point_2d {
    using coord_type = T;
    using const_point_reference = const point_2d<coord_type> &;
    using point_reference = point_2d<coord_type> &;
public:
    coord_type x_;
    coord_type y_;
public:
    point_2d(coord_type x, coord_type y);

    point_2d(const_point_reference point);

    point_2d() = default;

    point_reference operator=(const_point_reference point) &;

    bool operator==(const_point_reference point) const;

    friend std::istream &operator>><T>(std::istream &in, point_2d &coords);


};

/*Each line_2d have its coordinates of pt1_ and pt2_ in exactly
 * strict way for simplifying processing in different types of intersections and etc.
 * the pt1_ always have y_coordinates low than pt2_
 * and the case when the line is horizontal than pt1_ is on the left side of the line
 * it means that pt1_ have low x_coordinate value
 *
 *
 *
 * It's very important condition due to not following of it may cause errors*/

template<typename T>
class line_2d {
    //users changes of types
    using coord_type = T;
    using coord_ptr = T *;
    using point_type = point_2d<T>;
    using point_ptr = point_2d<T> *;
    using const_point_type = const point_2d<T>;
    using const_point_ptr = const point_2d<T> *;
    using const_point_reference = const point_2d<T> &;
    using const_line_reference = const line_2d<T> &;
    using line_reference = line_2d<T> &;
    //----------------------------------------------------------------------//
private:
    void Type_Coincedence(const_line_reference line2D, int *type_intersect) const;

    void Coincedence_In_Vertical_Case(const_line_reference line2D, int *type_intersect) const;

    point_ptr Pos_Neg_Area(const_line_reference line2D, int *type_intersect) const;

    point_ptr Type_Area_No_Intersection(const_line_reference line2D, int *type_intersect) const;

/*I've decided to make these fields public to have free access to them in different places*
 * Because i'm not going to think a lot about security in this program and i don't want to write getters and setters for e
 * each of these fields it will consume a lot of code and it will waste a lot of time*/
public:
    point_type pt1_;
    point_type pt2_;

    //necessary characteristic for each line in 2d
    point_type normal;
    point_type guiding;


    //the __ c __ field represents the coefficient in the vector form of the line segment like [ Dot(normal, x) = c ]; c = Dot(r_0, normal)
    coord_type c;

    /*this field became necessary in my realization, because i thought a lot about how to compensate right
     * counterclockwise direction of edges of polygons (it's very important to own such an information during
     * dealing with polygons, for example during boolean operations on polygons
     * So i've taken a decision to save this fact in this field and when i will need it i'll easily get the information
     * of initial direction of the line in polygon if the [is_counterclockwise == 1] it means that this line segment has the
     * primary counterclockwise direction in polygon otherwise his initial direction has been changed*/
    bool is_counterclockwise = true;

public:
    line_2d() = default;

    line_2d(const_point_reference point1, const_point_reference point2);

    line_2d(const_point_reference point1, const_point_reference point2, bool dir);

    line_2d(coord_type x1, coord_type y1, coord_type x2, coord_type y2);

    line_2d(const_line_reference line2D);

    line_reference operator=(const_line_reference line2D) &;

//    bool operator()(const_line_reference line1, const_line_reference line2)const;
    void check_order();

    void swap_order();

    //this argument will point to the spliting line so its coordinates will be the pt1, pt2
    point_ptr lines_intersection(const_line_reference line2D, int *type_intersect) const;

    //this method returns the vector product on given coordinates of vectors
    coord_type Kross(const_point_reference pt1, const_point_reference pt2) const;

    //this method returns the scalar product on given coordinates of vectors
    coord_type Dot(const_point_reference pt1, const_point_reference pt2) const;

    bool Is_Point_On_The_Line(const_point_type pt) const;


};


#endif //COUNT_INTERSECTION_AREA_OF_TRIANGLE_LINE_2D_H
