//
// Created by denis on 13.10.19.
//

#include "BSPTree.h"

using namespace bsp;

template<typename T>
typename BSPTree_Node<T>::node_ptr BSPTree_Node<T>::get_neg() {
    return neg_child;
}

template<typename T>
typename BSPTree_Node<T>::node_ptr BSPTree_Node<T>::get_pos() {
    return pos_child;
}


template<typename T>
BSPTree_Node<T>::BSPTree_Node(node_ptr pos, node_ptr neg, const_line_reference line2D) : pos_child(pos), neg_child(neg),
                                                                                         edge(line2D) {}

/*This constructor must fill the list of edges with right directed edges
 * such that each line's pt1 point has coordinate of y less than pt2 point
 *
 *
 * Some notes about this constructor :
 * 1) It takes a vector of point_2d points which represent the sequence of vertexes of polygon
 * 2) And the order of vertex can be arbitary either counterclockwise or clockwise order of vertexes can exist
 * 3) But this constructor fills the list of edges which will be used by the function of producing the bsp tree of a polygon
 * and the requirement for this constructor to understand whether the primary set of coordinates was counterclockwise or clockwise
 * and if 1) counterclockwise then just change direction of the edges following the pivotal rules of my realization of bsp tree for
 * polygons
 * 2) Change the primary direction of edges to the counterclockwise order and only then make necessary changing for creating the
 * list of edges with correct directions of edges*/

template<typename T>
BSPTree<T>::BSPTree(const std::vector <point_type> &coords) {
    /*To determine whether the direction of vertex is counterclockwise or clockwise i'm going to use following algorithm :
     * If you have only convex polygons (and all regular polygons are convex), and if your points are all
     * organized consistently--either all counterclockwise or all clockwise--then you can determine
     * which by just computing the (signed) area of one triangle determined
     * by any three consecutive points. This is essentially computing the
     * cross product of the two vectors along the two edges.*/
    coord_type double_signed_area = 0.0;
    int j = 0;
    for (int i = 0; i < 3; ++i) {
        j = (i + 1) % 3;
        double_signed_area += (coords[i].y_ + coords[j].y_) * (coords[j].x_ - coords[i].x_);
    }

    /*Here i want to remind that is_counterclockwise order field in line_2d just say
     * whether the line had initial counterclockwise order or it has been changed
     * if [it is equal to true] then the primary order is counterclockwise otherwise is clockwise*/

    assert(double_signed_area != 0);

    int number_coord = coords.size();
    line_type line_to_insert;
    if (double_signed_area > 0) {

        for (int i = number_coord; i > 0; --i) {
            j = (i - 1) % number_coord;
            line_to_insert = line_type(coords[i % number_coord], coords[j]);
            Insert_Edge(line_to_insert);
        }
    } else {
        for (int i = 0; i < number_coord; ++i) {
            j = (i + 1) % number_coord;
            line_to_insert = line_type(coords[i], coords[j]);
            Insert_Edge(line_to_insert);
        }
    }
}

template<typename T>
void BSPTree<T>::Insert_Edge(line_reference line2D) {
    line2D.check_order();
    edges.push_back(line2D);
}


template<typename T>
typename BSPTree<T>::node_ptr BSPTree<T>::Construct_Tree(iterator it, int pos_dist, int neg_dist, bool direction) {

    if (direction && pos_dist == 0)
        return nullptr;
    if (!direction && neg_dist < 0)
        return nullptr;

    auto it_begin = it;
    if (direction) {
        std::advance(it_begin, -pos_dist);
        neg_dist = pos_dist - 1;
        pos_dist = 0;
    } else {
        if (neg_dist != static_cast<int>(edges.size() - 1)) {
            ++it_begin;
            if (it_begin == edges.end())
                return nullptr;

        }

    }


    int type_intersection = 0;
    iterator current_edge = it_begin;

    if (neg_dist)
        ++current_edge;

    iterator last_edge;

    if (direction)
        last_edge = it;
    else {

        last_edge = std::next(current_edge, neg_dist);
    }

    point_ptr common_point;

    for (; current_edge != last_edge;) {
        common_point = it_begin->lines_intersection(*current_edge, type_intersection);

        //if type_intersection == Negative then we mustn't do anything with the line
        if (type_intersection == line_2d<T>::Positive) {
            //due to i'm going to erase the element by iterator current_edge, so i must save its copy to iterate further

            edges.insert(it_begin, *current_edge);
            ++current_edge;
            edges.erase(std::next(current_edge, -1));

            ++pos_dist;
            --neg_dist;
            delete common_point;
            continue;
        } else if (type_intersection == line_2d<coord_type>::Crossing ||
                   (type_intersection == line_2d<coord_type>::Without_Crossing && common_point)) {
            /*but the most important case in this condition when the second condition is true
             * and the first condition is very specifical it can arise when we deal
             * with very difficult polygon*/
            line_type first_part = line_type(current_edge->pt1_, *common_point);
            it_begin->lines_intersection(first_part, type_intersection);

            if (type_intersection == line_2d<coord_type>::Positive) {

                edges.insert(it_begin, first_part);
                current_edge->pt1_ = *common_point;
            } else if (type_intersection == line_2d<coord_type>::Negative) {
                edges.insert(it_begin, line_type(*common_point, current_edge->pt2_));
                current_edge->pt2_ = *common_point;

            }
            ++pos_dist;
            ++current_edge;
            delete common_point;
            continue;
        }


        delete common_point;
        ++current_edge;


        //once more case when two line are coincedence

    }
    --neg_dist;

    node_ptr pos_child = Construct_Tree(it_begin, pos_dist, neg_dist, true);

    node_ptr neg_child = Construct_Tree(it_begin, pos_dist, neg_dist, false);

    auto pivotal = new node_type(pos_child, neg_child, *it_begin);

    return pivotal;

}

template<typename T>
void BSPTree<T>::Make_Tree() {
    //______________Creating the bsp tree for polygon___________________//
    root__ = Construct_Tree(edges.begin(), 0, edges.size() - 1, false);

}

template<typename T>
void BSPTree<T>::Clear_Bsp_Tree(node_ptr edge) {
    if (!edge)
        return;

    Clear_Bsp_Tree(edge->get_neg());
    Clear_Bsp_Tree(edge->get_pos());

    delete edge;


}

template<typename T>
typename BSPTree_Node<T>::const_line_reference BSPTree_Node<T>::get_edge() const {
    return this->edge;
}

template<typename T>
void BSPTree<T>::Partitioning_Line_Segment(std::list <line_type> &inside_edges, const_line_reference line2D,
                                           node_ptr polygon_for_intersect) {
    if (!polygon_for_intersect)
        return;

    int type_intersection = 0;
    point_ptr common_point = (polygon_for_intersect->get_edge()).lines_intersection(line2D, type_intersection);

    switch (type_intersection) {

        case line_2d<coord_type>::Positive : {
            Partitioning_Line_Segment(inside_edges, line2D, polygon_for_intersect->get_pos());
            break;
        }


        case line_2d<coord_type>::Spliting_Line_Above :
        case line_2d<coord_type>::Spliting_Line_Below :
        case line_2d<coord_type>::Full_Inclusion_Of_Spliting_Line: {
            return;
        }


        case line_2d<coord_type>::Negative : {
            Partitioning_Line_Segment(inside_edges, line2D, polygon_for_intersect->get_neg());
            break;
        }

        case line_2d<coord_type>::Crossing_Without_Intersection :
        case line_2d<coord_type>::One_Point :
        case line_2d<coord_type>::Crossing : {

            line_type first_part(line2D.pt1_, *common_point, line2D.is_counterclockwise);
            line_type second_part(*common_point, line2D.pt2_, line2D.is_counterclockwise);

            if (line_tools::Dot(line2D.pt1_, (polygon_for_intersect->get_edge()).normal) -
                polygon_for_intersect->get_edge().c > 0) {

                Partitioning_Line_Segment(inside_edges, first_part, polygon_for_intersect->get_pos());
                Partitioning_Line_Segment(inside_edges, second_part, polygon_for_intersect->get_neg());

            } else {
                Partitioning_Line_Segment(inside_edges, second_part, polygon_for_intersect->get_pos());
                Partitioning_Line_Segment(inside_edges, first_part, polygon_for_intersect->get_neg());
            }

            if (!polygon_for_intersect->get_neg() && !polygon_for_intersect->get_pos()) {

                if (!Check_Position_Last_Line(inside_edges, first_part, polygon_for_intersect))
                    Check_Position_Last_Line(inside_edges, second_part, polygon_for_intersect);

                delete common_point;

                return;
            }
            break;
        }

        case line_2d<coord_type>::Full_Coincidence : {
            /*we must to block pushing edge of triangle in case of full coincidence when processing with
             * opposite order of triangles has already pushed this edge
             * and as usual a lot depend on the orientation of the line segment
             * We must remember that each line segment in inside_edges list has the right direction
             * so we will have to compare opposite oriented line segment if it hasn't got the initial
             * direction*/
            if (line2D.is_counterclockwise) {
                if (std::find(inside_edges.begin(), inside_edges.end(), line2D) == inside_edges.end())
                    inside_edges.push_back(line2D);
            } else {
                line_type opposite_oriented_edge(line2D.pt2_, line2D.pt1_);
                if (std::find(inside_edges.begin(), inside_edges.end(), opposite_oriented_edge) == inside_edges.end())
                    inside_edges.push_back(opposite_oriented_edge);
            }
            return;
        }

        case line_2d<coord_type>::Full_Inclusion_In_Spliting_Line : {
            Right_Insert(inside_edges, line2D);
            delete common_point;
            return;
        }

        case line_2d<coord_type>::Without_Crossing : {
            break;
        }

        default : {
            std::cout << "ERROR TYPE OF INTERSECTION!!!" << type_intersection << std::endl;
            delete common_point;
            return;
        }


    }
    if (!polygon_for_intersect->get_neg() && !polygon_for_intersect->get_pos()) {

        Check_Position_Last_Line(inside_edges, line2D, polygon_for_intersect);
    }

    delete common_point;
}

template<typename T>
void BSPTree<T>::Right_Insert(std::list<line_type> &inside_edges, const_line_reference line2D) {

    if (!line2D.is_counterclockwise) {
        line_type copy_line = line2D;
        copy_line.swap_order();
        inside_edges.push_back(copy_line);
    } else
        inside_edges.push_back(line2D);
}

template<typename T>
typename BSPTree<T>::node_ptr BSPTree<T>::get_root() const {
    return root__;
}

template<typename T>
bool BSPTree<T>::Check_Position_Last_Line(std::list <line_type> &inside_edges, const_line_reference line2D,
                                          node_ptr last_edge) {
    int type_pos = Somewhere;

    Check_Inside_Outside_Last_Edge(line2D.pt1_, type_pos, last_edge);

    if (type_pos == Outside) {
        return false;
    } else if (type_pos == Inside) {
        Right_Insert(inside_edges, line2D);

        return true;
    } else if (type_pos == On) {
        Check_Inside_Outside_Last_Edge(line2D.pt2_, type_pos, last_edge);
        if (type_pos == Outside)
            return false;
        if (type_pos == On || type_pos == Inside) {

            Right_Insert(inside_edges, line2D);

            return true;
        }

    }


    //consider the default behavior returning false --- as a mark of outside position of the line2D
    return false;
}

template<typename T>
void BSPTree<T>::Polygon_Polygon_Intersection(std::list <line_type> &common_lines,
                                              node_ptr polygon_which_intersect) {
    if (!polygon_which_intersect)
        return;

    Partitioning_Line_Segment(common_lines, polygon_which_intersect->get_edge(), root__);

    Polygon_Polygon_Intersection(common_lines, polygon_which_intersect->get_pos());

    Polygon_Polygon_Intersection(common_lines, polygon_which_intersect->get_neg());
}

template<typename T>
void BSPTree<T>::Point_Position_In_Polygon(const_point_reference point, node_ptr edge,
                                           int &type_pos) const {
    if (!edge) {
        type_pos = Outside;
        return;
    }

    coord_type check_position = Dot(point, (edge->get_edge()).normal) - (edge->get_edge()).c;

    //using the fact of initial counterclockwise direction of points
    if (!edge->get_neg() && !edge->get_pos()) {
        //the case when the point lie on the last line segment of given polygon
        if (check_position == 0) {
            type_pos = On;
            return;
        }
        Check_Inside_Outside_Last_Edge(point, type_pos, edge);
        return;
    }
    //point is situated in the area where normal vector lies
    if (check_position > 0) {
        Point_Position_In_Polygon(point, edge->get_pos(), type_pos);
    }

        //point is situated in the different area than normal lies
    else if (check_position < 0) {
        Point_Position_In_Polygon(point, edge->get_neg(), type_pos);
    } else if (check_position == 0 && point.y_ >= edge->get_edge().pt1_.y_ && point.y_ <= edge->get_edge().pt2_.y_) {

        if (edge->get_edge().pt1_.x_ < edge->get_edge().pt2_.x_) {
            if (point.x_ >= edge->get_edge().pt1_.x_ && point.x_ <= edge->get_edge().pt2_.x_) {
                type_pos = On;
                return;
            } else {
                type_pos = Outside;
                return;
            }
        } else {
            if (point.x_ >= edge->get_edge().pt2_.x_ && point.x_ <= edge->get_edge().pt1_.x_) {
                type_pos = On;
                return;
            } else {
                type_pos = Outside;
                return;
            }
        }
    } else {
        type_pos = Outside;
        return;
    }


}

template<typename T>
void BSPTree<T>::Check_Inside_Outside_Last_Edge(const_point_reference point, int &type_pos, node_ptr last_node) const {
    //use the fact that the initial direction of edges is counterclockwise

    if (last_node->get_edge().is_counterclockwise) {

        coord_type value = Dot(point, last_node->get_edge().normal) - last_node->get_edge().c;
        if (value < 0)
            type_pos = Inside;
        else if (value > 0)
            type_pos = Outside;
        else type_pos = On;
    } else {
        point_type normal_initial_dir(-(last_node->get_edge().normal.x_), -(last_node->get_edge().normal.y_));
        coord_type c_initial_value = Dot(normal_initial_dir, last_node->get_edge().pt2_);
        coord_type value = Dot(point, normal_initial_dir) - c_initial_value;
        if (value < 0)
            type_pos = Inside;
        else if (value > 0)
            type_pos = Outside;
        else type_pos = On;
    }

    //NO OTHER CANDIDATES   ;) ;) ;)
}