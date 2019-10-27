//
// Created by denis on 13.10.19.
//

#include "line_2d.h"

using namespace line_tools;

//---------------implementations of point_2d class methods-------------------------------------//
template<typename T>
point_2d<T>::point_2d(coord_type x, coord_type y) : x_(x), y_(y) {}

template<typename T>
bool point_2d<T>::operator==(const_point_reference point) const {

    if (std::fabs(point.x_ - x_) <=
        std::numeric_limits<coord_type>::epsilon() * std::fabs(point.x_ + x_) * CORRECTION &&
        std::fabs(point.y_ - y_) <= std::numeric_limits<coord_type>::epsilon() * std::fabs(point.y_ + y_) * CORRECTION)
        return true;

    else if (std::fabs(point.x_ - x_) < std::numeric_limits<coord_type>::min() &&
             std::fabs(point.y_ - y_) < std::numeric_limits<coord_type>::min())
        return true;


    return false;

}

template<typename T>
typename point_2d<T>::point_reference point_2d<T>::operator=(const_point_reference point) &{
    if (this == &point)
        return *this;

    x_ = point.x_;
    y_ = point.y_;
    return *this;

}
//-----------------------------------------------------------------------------------------------//


//_________________HELPER FUNCTIONS FOR CLASS REPRESENTING LINE IN 2D____________________________//
template<typename T>
inline T line_tools::Kross(const point_2d<T> &vec1, const point_2d<T> &vec2) {
    return (vec1.x_ * vec2.y_) - (vec1.y_ * vec2.x_);
}

template<typename T>
inline T line_tools::Dot(const point_2d<T> &vec1, const point_2d<T> &vec2) {
    return vec1.x_ * vec2.x_ + vec1.y_ * vec2.y_;
}

void line_tools::Error_Message() {
    std::cout << "Error in the " << __LINE__ << " line" << std::endl;
    std::cout << "The file name is : " << __FILE__ << std::endl;
    exit(EXIT_FAILURE);
}
//-------------------------------------------------------------------------------------------------//

//_____________________Implementations of line_2d class methods_____________________________________//
template<typename T>
bool line_2d<T>::operator==(const_line_reference line2D) const {
    if (line2D.pt1_ == pt1_ && line2D.pt2_ == pt2_)
        return true;

    return false;
}

template<typename T>
typename line_2d<T>::point_ptr
line_2d<T>::Type_Area_No_Intersection(const_line_reference line2D, int &type_intersect) const {
    if (Dot(normal, line2D.pt1_) - c > 0 && Dot(normal, line2D.pt2_) - c > 0) {
        type_intersect = Positive;
        return nullptr;
    } else if (Dot(normal, line2D.pt1_) - c < 0 && Dot(normal, line2D.pt2_) - c < 0) {
        type_intersect = Negative;
        return nullptr;
    } else if ((Dot(normal, line2D.pt1_) - c == 0 && Dot(normal, line2D.pt2_) - c > 0) ||
               (Dot(normal, line2D.pt2_) - c == 0 && Dot(normal, line2D.pt1_) - c > 0)) {
        type_intersect = Positive;
        return nullptr;
    } else if ((Dot(normal, line2D.pt1_) - c == 0 && Dot(normal, line2D.pt2_) - c < 0) ||
               (Dot(normal, line2D.pt2_) - c == 0 && Dot(normal, line2D.pt1_) - c < 0)) {
        type_intersect = Negative;
        return nullptr;
    } else {

        //the critical section dividing by zero can appear, so i must think about another better desicion


        if (normal.x_ == 0) {
            /*the case when the normal is perpendicular to x axis, so we know exactly y_coord of the
            * common_point in intersection of these two line segments*/
            coord_type y_coord = pt1_.y_;
            if (line2D.normal.x_ == 0)
                Error_Message();
            coord_type x_coord = (line2D.c - y_coord * line2D.normal.y_) / line2D.normal.x_;
            auto point_in_intersection_lines = new point_type(x_coord, y_coord);
            type_intersect = Crossing_Without_Intersection;
            return point_in_intersection_lines;

        }

        coord_type check_error = line2D.normal.y_ * normal.x_ - normal.y_ * line2D.normal.x_;
        if (check_error == 0)
            Error_Message();

        coord_type y_coord = (line2D.c * normal.x_ - c * line2D.normal.x_) / check_error;

        coord_type x_coord = c / normal.x_ - (normal.y_ / normal.x_) * y_coord;


        auto point_in_intersection_lines = new point_type(x_coord, y_coord);
        type_intersect = Crossing_Without_Intersection;
        return point_in_intersection_lines;
    }
}

template<typename T>
typename line_2d<T>::point_ptr line_2d<T>::lines_intersection(const_line_reference line2D, int &type_intersect) const {
    //First line is pt1 and pt2

    coord_type vector_left = Kross(point_type(line2D.pt1_.x_ - pt1_.x_, line2D.pt1_.y_ - pt1_.y_),
                                   point_type(pt2_.x_ - pt1_.x_, pt2_.y_ - pt1_.y_));

    coord_type vector_right = Kross(point_type(line2D.pt2_.x_ - pt1_.x_, line2D.pt2_.y_ - pt1_.y_),
                                    point_type(pt2_.x_ - pt1_.x_, pt2_.y_ - pt1_.y_));

    // one of this value will be 0 it means that points lie exactly on the line
    coord_type vector_pr_first = vector_left * vector_right;
    if (vector_pr_first > 0) {
        return Type_Area_No_Intersection(line2D, type_intersect);
    }

    coord_type another_vector_left = Kross(point_type(pt1_.x_ - line2D.pt2_.x_, pt1_.y_ - line2D.pt2_.y_),
                                           point_type(line2D.pt1_.x_ - line2D.pt2_.x_,
                                                      line2D.pt1_.y_ - line2D.pt2_.y_));

    coord_type another_vector_right = Kross(point_type(pt2_.x_ - line2D.pt2_.x_, pt2_.y_ - line2D.pt2_.y_),
                                            point_type(line2D.pt1_.x_ - line2D.pt2_.x_,
                                                       line2D.pt1_.y_ - line2D.pt2_.y_));

    coord_type vector_pr_second = another_vector_left * another_vector_right;
    if (vector_pr_second > 0) {
        return Type_Area_No_Intersection(line2D, type_intersect);
    }


    //case when two lines are coincidence
    if (vector_left == 0 && vector_right == 0 && another_vector_left == 0 && another_vector_right == 0) {
        Type_Coincedence(line2D, type_intersect);
        return nullptr;
    }

    //we are in here so the lines exactly have the common point !!!
    //check case when the line2D to the positive side of spliting line
    if ((vector_left == 0 && vector_right != 0) || (vector_left != 0 && vector_right == 0)) {
        auto common_point = Pos_Neg_Area(line2D, type_intersect);
        if (!common_point) {
            std::cout << "ERROR!!! in finding the common point in touch case\n";
            exit(EXIT_FAILURE);
        }
        return common_point;

    }
    //not do anything
    if ((another_vector_left == 0 && another_vector_right != 0) ||
        (another_vector_right == 0 && another_vector_left != 0)) {
        point_ptr common_point;
        if (Dot(pt1_, line2D.normal) - line2D.c == 0) {
            common_point = new point_type(pt1_);
        } else
            common_point = new point_type(pt2_);
        type_intersect = One_Point;
        return common_point;
    }


    //find the coordinates of the intersection
    coord_type frac_vector_product = -(vector_left / vector_right); //each case it must be positive value !!!
    coord_type x_numerator = line2D.pt1_.x_ - line2D.pt2_.x_;
    coord_type denominator = 1 + frac_vector_product;

    //------------------------------------------//
    coord_type y_numerator = line2D.pt1_.y_ - line2D.pt2_.y_;


    if (denominator == 0)
        Error_Message();

    coord_type x_coord = x_numerator / denominator + line2D.pt2_.x_;
    coord_type y_coord = y_numerator / denominator + line2D.pt2_.y_;

    if ((x_coord > 0 && x_coord <= NUL_LIMIT) || (x_coord < 0 && x_coord >= -NUL_LIMIT))
        x_coord = 0.0;
    if ((y_coord > 0 && y_coord <= NUL_LIMIT) || (y_coord < 0 && y_coord >= -NUL_LIMIT))
        y_coord = 0.0;

    auto common_point_ptr = new point_type(x_coord, y_coord);
    type_intersect = Crossing;

    return common_point_ptr;
}

template<typename T>
void line_2d<T>::Type_Coincedence(const_line_reference line2D, int &type_intersect) const {

    //case of full coincidence of two line segments
    if (line2D.pt2_ == pt2_ && line2D.pt1_ == pt1_) {
        type_intersect = Full_Coincidence;
        return;

    }
    //case when the spliting line is vertical (very different case)
    if (Dot(guiding, {1, 0}) == 0) {
        Coincedence_In_Vertical_Case(line2D, type_intersect);
        return;
    }
    if (line2D.pt1_ == pt1_ && line2D.pt2_ == pt2_) {
        type_intersect = Full_Inclusion_Of_Spliting_Line;
        return;
    }

    coord_type low_limit = 0;
    coord_type high_limit = 0;
    coord_type low_limit_line2D = 0;
    coord_type high_limit_line2D = 0;

    bool direction = false;
    if (pt1_.x_ < pt2_.x_) {
        low_limit = pt1_.x_;
        high_limit = pt2_.x_;
        low_limit_line2D = line2D.pt1_.x_;
        high_limit_line2D = line2D.pt2_.x_;
        direction = true;
    } else {
        low_limit = pt2_.x_,
        high_limit = pt1_.x_,
        low_limit_line2D = line2D.pt2_.x_,
        high_limit_line2D = line2D.pt1_.x_;
    }


    //case when line2D fully within the spliting line
    if (line2D.pt1_.x_ >= low_limit && line2D.pt1_.x_ <= high_limit &&
        line2D.pt2_.x_ >= low_limit && line2D.pt2_.x_ <= high_limit) {
        type_intersect = Full_Inclusion_In_Spliting_Line;
        return;
    }

    //case when line2D fully contains the spliting line

    if (pt1_.x_ >= low_limit_line2D && pt2_.x_ <= high_limit_line2D &&
        pt1_.x_ <= high_limit_line2D && pt2_.x_ >= low_limit_line2D) {
        type_intersect = Full_Inclusion_Of_Spliting_Line;
        return;
    }

    //case when spliting line is above the line2D
    if ((low_limit < low_limit_line2D && low_limit_line2D <= high_limit && high_limit <= high_limit_line2D &&
         !direction) ||
        (low_limit_line2D <= low_limit && low_limit <= high_limit_line2D && high_limit_line2D < high_limit &&
         direction)) {
        type_intersect = Spliting_Line_Above;
        return;
    }

    //case when spliting line is below the line2D
    if ((low_limit_line2D < low_limit && low_limit <= high_limit_line2D && high_limit_line2D <= high_limit &&
         !direction) ||
        (low_limit <= low_limit_line2D && low_limit_line2D <= high_limit && high_limit < high_limit_line2D &&
         direction)) {
        type_intersect = Spliting_Line_Below;
        return;
    } else {
        type_intersect = Without_Crossing;
        return;
    }
}

template<typename T>
void line_2d<T>::Coincedence_In_Vertical_Case(const_line_reference line2D, int &type_intersect) const {
    if (line2D.pt2_ == pt2_ && line2D.pt1_ == pt1_) {
        type_intersect = Full_Inclusion_Of_Spliting_Line;
        return;
    }

    if (line2D.pt1_.y_ >= pt1_.y_ && line2D.pt2_.y_ >= pt1_.y_ &&
        line2D.pt1_.y_ <= pt2_.y_ && line2D.pt2_.y_ <= pt2_.y_) {
        type_intersect = Full_Inclusion_In_Spliting_Line;
        return;
    }
    if (pt1_.y_ >= line2D.pt1_.y_ && pt1_.y_ >= line2D.pt2_.y_ &&
        pt2_.y_ <= line2D.pt1_.y_ && pt2_.y_ <= line2D.pt2_.y_) {
        type_intersect = Full_Inclusion_Of_Spliting_Line;
        return;
    }

    if (line2D.pt2_.y_ > pt2_.y_ && line2D.pt1_.y_ <= pt2_.y_) {
        type_intersect = Spliting_Line_Below;
        return;
    }

    if (pt2_.y_ > line2D.pt2_.y_ && pt1_.y_ <= line2D.pt2_.y_) {
        type_intersect = Spliting_Line_Above;
    } else {
        type_intersect = Without_Crossing;
        return;
    }

}

template<typename T>
line_2d<T>::line_2d(const_point_reference point1, const_point_reference point2) :
        pt1_(point1),
        pt2_(point2) {
    guiding = {pt2_.x_ - pt1_.x_, pt2_.y_ - pt1_.y_};
    normal = {guiding.y_, -guiding.x_};
    c = Dot(pt1_, normal);
}


template<typename T>
line_2d<T>::line_2d(const_point_reference point1, const_point_reference point2, bool dir) : line_2d(point1, point2) {
    is_counterclockwise = dir;
}

template<typename T>
typename line_2d<T>::line_reference line_2d<T>::operator=(const_line_reference line2D) &{
    if (this == &line2D)
        return *this;

    pt1_ = line2D.pt1_;
    pt2_ = line2D.pt2_;
    normal = line2D.normal;
    guiding = line2D.guiding;
    c = line2D.c;
    is_counterclockwise = line2D.is_counterclockwise;
    return *this;

}

template<typename T>
line_2d<T>::line_2d(coord_type x1, coord_type y1, coord_type x2, coord_type y2) :
        pt1_(x1, y1), pt2_(x2, y2) {
    guiding = {pt2_.x_ - pt1_.x_, pt2_.y_ - pt1_.y_};
    normal = {guiding.y_, -guiding.x_};
    c = Dot(pt1_, normal);
}

template<typename T>
line_2d<T>::line_2d(const_line_reference line2D) : pt1_(line2D.pt1_), pt2_(line2D.pt2_), normal(line2D.normal),
                                                   guiding(line2D.guiding), c(line2D.c),
                                                   is_counterclockwise(line2D.is_counterclockwise) {}


template<typename T>
bool line_2d<T>::Is_Point_On_The_Line(line_2d::const_point_type pt) const {
    if (pt.x_ * normal.x_ + pt.y_ * normal.y_ == c)
        return true;
    else return false;
}

template<typename T>
typename line_2d<T>::point_ptr line_2d<T>::Pos_Neg_Area(const_line_reference line2D, int &type_intersect) const {
    point_ptr common_point;
    if (Is_Point_On_The_Line(line2D.pt1_)) {
        common_point = new point_type(line2D.pt1_);
        if ((Dot(line2D.pt2_, normal) - c) > 0)
            type_intersect = Positive;
        else
            type_intersect = Negative;
    } else if (Is_Point_On_The_Line(line2D.pt2_)) {
        common_point = new point_type(line2D.pt2_);
        if ((Dot(line2D.pt1_, normal) - c) > 0)
            type_intersect = Positive;
        else
            type_intersect = Negative;

    } else
        common_point = nullptr;

    return common_point;
}

template<typename T>
void line_2d<T>::check_order() {
    if (pt1_.y_ > pt2_.y_)
        swap_order();

    if (pt1_.y_ == pt2_.y_ && pt1_.x_ > pt2_.x_)
        swap_order();
}

template<typename T>
void line_2d<T>::swap_order() {
    point_type tempr = pt1_;
    pt1_ = pt2_;
    pt2_ = tempr;
    //change the direction of pivotal vectors and constant c
    normal.y_ = -normal.y_;
    normal.x_ = -normal.x_;
    guiding.x_ = -guiding.x_;
    guiding.y_ = -guiding.y_;
    c = Dot(normal, pt1_);
    is_counterclockwise = !is_counterclockwise;
}