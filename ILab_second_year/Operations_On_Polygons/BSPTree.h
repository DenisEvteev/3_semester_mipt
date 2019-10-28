//
// Created by denis on 13.10.19.
//

#ifndef COUNT_INTERSECTION_AREA_OF_TRIANGLE_BSPTREE_H
#define COUNT_INTERSECTION_AREA_OF_TRIANGLE_BSPTREE_H

#include "line_2d.h"
#include <list>
#include <vector>

using namespace line_tools;

namespace bsp {

    enum Point_Position {
        Somewhere,
        Inside,
        Outside,
        On
    };

    template<typename T>
    class BSPTree_Node {
        using const_line_reference = const line_2d<T> &;
        using node_ptr = BSPTree_Node *;

        node_ptr pos_child;
        node_ptr neg_child;
        line_2d<T> edge;

    public:
        //of course i must write these methods for const objects, but not now !!!!!!!!!
        node_ptr get_pos();

        node_ptr get_neg();

        const_line_reference get_edge() const;

        BSPTree_Node(node_ptr pos, node_ptr neg, const_line_reference line2D);

    };


    template<typename T>
    class BSPTree {
        using line_type = line_2d<T>;
        using const_line_reference = const line_2d<T> &;
        using line_reference = line_2d<T> &;
        using coord_type = T;
        using node_ptr = BSPTree_Node<coord_type> *;
        using node_type = BSPTree_Node<coord_type>;
        using point_type = point_2d<T>;
        using const_point_reference = const point_2d<T> &;
        using point_ptr = point_2d<T> *;
        using iterator = typename std::list<line_type>::iterator;
        using c_iterator = typename std::list<line_type>::const_iterator;

    public:

        node_ptr get_root() const;

        //this constructor will accept the vector of set of coordinates and prepare it for creating the bsp tree of a polygon
        explicit BSPTree(const std::vector<point_2d<T>> &coords);

        void Polygon_Polygon_Intersection(std::list <line_type> &inside_edges, node_ptr polygon_which_intersect);

        void Partitioning_Line_Segment(std::list <line_type> &inside_edges, const_line_reference line2D,
                                       node_ptr polygon_for_intersect);

        void Make_Tree();

        void Clear_Bsp_Tree(node_ptr edge);

        /*my realization of bsp tree for polygons gives some disadvantages due to determination whether
         * the point is inside, outside or on the polygon
         * My realization of bsp tree cannot say exactly what are the areas on which line segments divide the plane
         * It's not very nice. So i must come up with the idea to determine the position of the point in relative to the
         * last edge.
         * Yeah, i had the difficult choice between time complexity (my realization allow to create balanced bsp tree
         * because we usually have both directions of edges in polygons so we will have  the tree with less depth
         * than when we create it using the counterclockwise or clockwise representation of the tree) and the
         * crutch which i had to develop.
         * Using this program you must give the program in input points in counterclockwise direction
         * Then constructor of BSPTree will change the order of coordinates for decreasing of depth of bsp tree
         *
         *
         * Okay, and pivotal thought here this is that the function Point_Position_In_Polygon
         * will use the fact that initial direction of points in polygon was the counterclockwise
         * and for checking the position of points with the last edge checking whether the point is inside or outside the polygon
         * convert the coordinates of the last edge and check then a position of a point*/


        //It's so nice decision i think
        void Point_Position_In_Polygon(const_point_reference point, node_ptr edge, int &type_pos) const;

        void Check_Inside_Outside_Last_Edge(const_point_reference point, int &type_pos, node_ptr last_node) const;

        //This function returns true if it has added the element in list otherwise false
        bool
        Check_Position_Last_Line(std::list <line_type> &inside_edges, const_line_reference line2D, node_ptr last_edge);

    private:

        node_ptr root__ = nullptr;


        /*in my realization of bsp tree i'm going to fill the [edge list] with lines -- aka edges of polygon
         * in a quite special way --- each line will have the first and second point in right order, i mean that :
         * 1) the pt1_ of line must have the lower y_coordinate than pt2_ point
         * 2) if the line is horizontal the pt1_ must be on the left side than pt2_ point
         * these conditions will be made during construction of edge and they must be true for each edge
         * ___It will simplify the processing in different cases
         * It seems to be not very understandable but i've decided to do like this in this realization*/

        std::list <line_type> edges;   //the list that contains all the edges of a polygon

        //This is a recursive function for constructing the bsp tree for a polygon
        node_ptr Construct_Tree(iterator it, int pos_dist, int neg_dist, bool direction);

        void Right_Insert(std::list<line_type> &inside_edges, const_line_reference line2D);

        void Insert_Edge(line_reference line2D);


    };

}


#endif //COUNT_INTERSECTION_AREA_OF_TRIANGLE_BSPTREE_H
